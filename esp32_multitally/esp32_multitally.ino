// ====================== HARD SETTINGS ======================
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

// --- WiFi ---
#define WIFI_SSID       "camera_server"
#define WIFI_PASSWORD   "Ferarinomor123"

// --- Feelworld (LivePro/SE) ---
#define FEELWORLD_IP    "192.168.1.121"   // <— set to your LivePro’s IP
#define FEELWORLD_PORT  1000             // typical Feelworld control port

// --- Local UDP port to listen on (any free port) ---
#define UDP_LOCAL_PORT  12345

// --- Your tally camera number (1..4) ---
#define MY_CAMERA_INDEX 1

// --- LED strip/pixel ---
#define LED_PIN         2
#define NUM_LEDS        1
#define LED_PIXEL_TYPE  (NEO_GRB + NEO_KHZ800)  // try NEO_RGB if colors wrong

// --- Timing ---
#define CONNECTION_TIMEOUT    800   // ms wait per request
#define STATUS_UPDATE_INTERVAL 800  // ms between polls

// --- Debugging ---
#define SERIAL_DEBUG 1

// ====================== CODE START ======================
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, LED_PIXEL_TYPE);
WiFiUDP udp;

bool wifiConnected        = false;
bool feelworldConnected   = false;
bool connectedAckOnce     = false;

uint8_t seqNum            = 0;

int previewIndex = -1;  // 0..3 or -1 unknown
int liveIndex    = -1;  // 0..3 or -1 unknown

// ---------- helpers ----------
static inline String b2hex(uint8_t v) {
  char buf[3];
  sprintf(buf, "%02X", v);
  return String(buf);
}

static void hexDump(const uint8_t* data, int len) {
  for (int i = 0; i < len; ++i) {
    if (i && (i % 16 == 0)) Serial.println();
    Serial.print(b2hex(data[i])); Serial.print(' ');
  }
  Serial.println();
}

static void setLED(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

static void updateLEDDecision() {
  if (!wifiConnected) { setLED(0,0,0); return; }
  if (!feelworldConnected) { setLED(128,0,128); return; } // purple
  if (previewIndex < 0 && liveIndex < 0) { setLED(0,0,255); return; } // blue

  const int me = MY_CAMERA_INDEX - 1; // convert to 0..3
  bool mineLive = (liveIndex == me);
  bool minePrev = (previewIndex == me);

  if (mineLive) setLED(255,0,0);      // RED
  else if (minePrev) setLED(0,255,0); // GREEN
  else setLED(0,0,0);                 // OFF

#if SERIAL_DEBUG
  Serial.printf("LED decision -> me=%d, preview=%d, live=%d, LED=%s\n",
    MY_CAMERA_INDEX, (previewIndex>=0?previewIndex+1:-1), (liveIndex>=0?liveIndex+1:-1),
    mineLive ? "RED" : (minePrev ? "GREEN" : "OFF"));
#endif
}

// ---------- Feelworld protocol frame ----------
struct FMsg {
  bool transmit;
  uint8_t addr;
  uint8_t seq;
  uint8_t cmd;
  uint8_t d1, d2, d3, d4;
  String toString() const {
    uint8_t sum = (addr + seq + cmd + d1 + d2 + d3 + d4) & 0xFF;
    String s = "<";
    s += transmit ? "T" : "F";
    auto H=[&](uint8_t v){ return b2hex(v); };
    s += H(addr)+H(seq)+H(cmd)+H(d1)+H(d2)+H(d3)+H(d4)+H(sum);
    s += ">";
    return s;
  }
};

static bool isAsciiAck(const uint8_t* buf, int len) {
  if (len != 19) return false;
  if (buf[0] != '<' || buf[18] != '>') return false;
  if (buf[1] != 'F' && buf[1] != 'T') return false;
  // Rough sanity: all bytes printable
  for (int i=0;i<len;i++) if (buf[i] < 0x20 || buf[i] > 0x7E) return false;
  return true;
}

static void logAsciiAck(const uint8_t* buf) {
  // <F AADD CC DD EE FF GG HH SS>
  // positions: [0:'<'][1:'F/T'][2..3:addr][4..5:seq][6..7:cmd][8..9:d1][10..11:d2][12..13:d3][14..15:d4][16..17:sum][18:'>']
  auto hexPair = [&](int i){ char z[3]; z[0]=buf[i]; z[1]=buf[i+1]; z[2]=0; return String(z); };
  uint8_t addr = strtol(hexPair(2).c_str(),  nullptr, 16);
  uint8_t seq  = strtol(hexPair(4).c_str(),  nullptr, 16);
  uint8_t cmd  = strtol(hexPair(6).c_str(),  nullptr, 16);
  uint8_t d1   = strtol(hexPair(8).c_str(),  nullptr, 16);
  uint8_t d2   = strtol(hexPair(10).c_str(), nullptr, 16);
  uint8_t d3   = strtol(hexPair(12).c_str(), nullptr, 16);
  uint8_t d4   = strtol(hexPair(14).c_str(), nullptr, 16);
  uint8_t sum  = strtol(hexPair(16).c_str(), nullptr, 16);
  uint8_t calc = (addr + seq + cmd + d1 + d2 + d3 + d4) & 0xFF;

  Serial.printf("ACK ASCII: addr=%02X seq=%02X cmd=%02X d1=%02X d2=%02X d3=%02X d4=%02X sum=%02X (calc=%02X)\n",
                addr, seq, cmd, d1, d2, d3, d4, sum, calc);
}

// ---------- WiFi ----------
static void connectWiFi() {
#if SERIAL_DEBUG
  Serial.print("Connecting WiFi: "); Serial.println(WIFI_SSID);
#endif
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(300);
    Serial.print('.');
  }
  Serial.println();

  wifiConnected = (WiFi.status() == WL_CONNECTED);
#if SERIAL_DEBUG
  Serial.println(wifiConnected ? "WiFi OK" : "WiFi FAILED");
  if (wifiConnected) { Serial.print("IP: "); Serial.println(WiFi.localIP()); }
#endif
  if (wifiConnected) udp.begin(UDP_LOCAL_PORT);

  updateLEDDecision();
}

// ---------- Feelworld connect (ASCII ack expected) ----------
static void feelworldConnect() {
  if (!wifiConnected) return;

  FMsg m;
  m.transmit = true;
  m.addr = 0x00;
  m.seq = seqNum++;
  m.cmd = 0x68;     // CONNECT
  m.d1  = 0x66;     // CONNECT_SUB
  m.d2 = m.d3 = m.d4 = 0x00;

  String out = m.toString();
#if SERIAL_DEBUG
  Serial.print("CONNECT send: "); Serial.println(out);
#endif
  udp.beginPacket(FEELWORLD_IP, FEELWORLD_PORT);
  udp.write((const uint8_t*)out.c_str(), out.length());
  udp.endPacket();

  unsigned long start = millis();
  bool gotAck = false;
  while (millis() - start < CONNECTION_TIMEOUT) {
    int ps = udp.parsePacket();
    if (ps) {
      uint8_t buf[256];
      int n = udp.read(buf, sizeof(buf));
      Serial.printf("CONNECT recv (%d bytes): ", n); hexDump(buf, n);
      if (isAsciiAck(buf, n)) {
        logAsciiAck(buf);
        gotAck = true;
        break;
      }
    }
    delay(20);
  }
  feelworldConnected = gotAck;
  connectedAckOnce = gotAck;
#if SERIAL_DEBUG
  Serial.println(feelworldConnected ? "Feelworld connected (ACK OK)" : "Feelworld connect timeout");
#endif
  updateLEDDecision();
}

// ---------- Status polling (ASCII ack + BINARY state) ----------
static void pollStatus() {
  if (!wifiConnected) return;

  // Build status request: F1 / sub=40 "selected", d2='01', d3='00', d4='00'
  FMsg m;
  m.transmit = true;
  m.addr = 0x00;
  m.seq  = seqNum++;
  m.cmd  = 0xF1;   // status
  m.d1   = 0x40;   // selected
  m.d2   = 0x01;   // request set 01 (matches your Dart)
  m.d3   = 0x00;
  m.d4   = 0x00;

  String out = m.toString();
#if SERIAL_DEBUG
  Serial.print("STATUS send: "); Serial.println(out);
#endif
  udp.beginPacket(FEELWORLD_IP, FEELWORLD_PORT);
  udp.write((const uint8_t*)out.c_str(), out.length());
  udp.endPacket();

  // Expect up to TWO packets: 1) ASCII ack, 2) BINARY state
  bool gotAck = false;
  bool gotState = false;
  unsigned long start = millis();
  int packets = 0;

  while (millis() - start < CONNECTION_TIMEOUT && packets < 3) {
    int ps = udp.parsePacket();
    if (!ps) { delay(10); continue; }
    uint8_t buf[256];
    int n = udp.read(buf, sizeof(buf));
    packets++;

    Serial.printf("STATUS recv #%d (%d bytes): ", packets, n);
    hexDump(buf, n);

    if (isAsciiAck(buf, n)) {
      logAsciiAck(buf);
      gotAck = true;
      feelworldConnected = true;
      continue; // look for next (binary) packet
    }

    // Treat as BINARY STATE packet
    if (n >= 3) {
      // Dart uses: preview = resp[0]; live = resp[2];
      previewIndex = buf[0]; // 0..3
      liveIndex    = buf[2]; // 0..3
      gotState = true;

#if SERIAL_DEBUG
      Serial.printf("Parsed BINARY state -> preview=%d (cam %d), live=%d (cam %d)\n",
        previewIndex, (previewIndex>=0?previewIndex+1:-1),
        liveIndex, (liveIndex>=0?liveIndex+1:-1));
#endif
      break; // we got what we need
    } else {
      Serial.println("Binary payload too short; ignoring.");
    }
  }

  if (!gotAck) {
    // If never got an ack, we’re likely disconnected
    feelworldConnected = false;
  }

  updateLEDDecision();
}

// ====================== Arduino ======================
void setup() {
#if SERIAL_DEBUG
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== ESP32 Feelworld Tally ===");
#endif

  strip.begin();
  strip.clear();
  strip.show();

  connectWiFi();
  feelworldConnect();
}

void loop() {
  if (!wifiConnected) {
    connectWiFi();
    delay(1000);
    return;
  }
  if (!feelworldConnected) {
    feelworldConnect();
    delay(1000);
    return;
  }

  pollStatus();
  delay(STATUS_UPDATE_INTERVAL);
}
