#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "config.h"

WiFiUDP udp;
unsigned int sequenceNumber = 0;
bool connected = false;

// Feelworld Protocol Commands
enum FCommand {
  CONNECT = 0x68,
  STATUS = 0xF1
};

enum FSubCommand {
  CONNECT_SUB = 0x66,
  STATUS_SELECTED = 0x40
};

struct FMessage {
  bool transmit;
  int address;
  int sequence;
  int command;
  int dat1;
  int dat2;
  int dat3;
  int dat4;
  
  String toString() {
    String msg = "<";
    msg += transmit ? "T" : "F";
    msg += byteToHex(address);
    msg += byteToHex(sequence);
    msg += byteToHex(command);
    msg += byteToHex(dat1);
    msg += byteToHex(dat2);
    msg += byteToHex(dat3);
    msg += byteToHex(dat4);
    
    int sum = address + sequence + command + dat1 + dat2 + dat3 + dat4;
    msg += byteToHex(sum);
    msg += ">";
    
    return msg;
  }
  
  String byteToHex(int value) {
    String hex = String(value & 0xFF, HEX);
    if (hex.length() == 1) hex = "0" + hex;
    return hex;
  }
};

void setup() {
  if (SERIAL_DEBUG) Serial.begin(115200);
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  connectToWiFi();
  connectToFeelworld();
}

void loop() {
  if (connected) {
    updateCameraStatus();
    delay(STATUS_UPDATE_INTERVAL);
  } else {
    // Attempt to reconnect
    connectToFeelworld();
    delay(5000);
  }
}

void connectToWiFi() {
  if (SERIAL_DEBUG) {
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (SERIAL_DEBUG) Serial.print(".");
  }
  
  if (SERIAL_DEBUG) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void connectToFeelworld() {
  if (SERIAL_DEBUG) Serial.println("Connecting to Feelworld device...");
  
  FMessage connectMsg;
  connectMsg.transmit = true;
  connectMsg.address = 0;
  connectMsg.sequence = sequenceNumber++;
  connectMsg.command = CONNECT;
  connectMsg.dat1 = CONNECT_SUB;
  connectMsg.dat2 = 0;
  connectMsg.dat3 = 0;
  connectMsg.dat4 = 0;
  
  String message = connectMsg.toString();
  if (SERIAL_DEBUG) {
    Serial.print("Sending: ");
    Serial.println(message);
  }
  
  udp.beginPacket(FEELWORLD_IP, FEELWORLD_PORT);
  udp.write(message.c_str());
  udp.endPacket();
  
  // Wait for response
  unsigned long startTime = millis();
  while (millis() - startTime < CONNECTION_TIMEOUT) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char buffer[256];
      int len = udp.read(buffer, 255);
      if (len > 0) {
        buffer[len] = 0;
        if (SERIAL_DEBUG) {
            Serial.print("Received: ");
            Serial.println(buffer);
        }
        
        // Parse response
        if (String(buffer).startsWith("<F")) {
          connected = true;
          if (SERIAL_DEBUG) Serial.println("Connected to Feelworld device!");
          return;
        }
      }
    }
    delay(100);
  }
  
  if (SERIAL_DEBUG) Serial.println("Connection timeout");
  connected = false;
}

void updateCameraStatus() {
  FMessage statusMsg;
  statusMsg.transmit = true;
  statusMsg.address = 0;
  statusMsg.sequence = sequenceNumber++;
  statusMsg.command = STATUS;
  statusMsg.dat1 = STATUS_SELECTED;
  statusMsg.dat2 = 1;  // Requesting status for selected cameras
  statusMsg.dat3 = 0;
  statusMsg.dat4 = 0;
  
  String message = statusMsg.toString();
  if (SERIAL_DEBUG) {
    Serial.print("Requesting status: ");
    Serial.println(message);
  }
  
  udp.beginPacket(FEELWORLD_IP, FEELWORLD_PORT);
  udp.write(message.c_str());
  udp.endPacket();
  
  // Wait for response
  unsigned long startTime = millis();
  while (millis() - startTime < CONNECTION_TIMEOUT) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char buffer[256];
      int len = udp.read(buffer, 255);
      if (len > 0) {
        buffer[len] = 0;
        if (SERIAL_DEBUG) {
            Serial.print("Status response: ");
            Serial.println(buffer);
        }
        
        // Parse camera status from response
        parseCameraStatus(buffer);
        return;
      }
    }
    delay(100);
  }
  
  if (SERIAL_DEBUG) Serial.println("Status request timeout");
  connected = false;
}

void parseCameraStatus(const char* response) {
  // Response format: <Fxxxxxxxxxx> where bytes represent camera states
  String resp = String(response);
  
  if (resp.length() >= 19 && resp.startsWith("<F")) {
    // dat1 (byte 9-10) is preview camera, dat2 (byte 11-12) is live camera
    int previewCam = hexToByte(resp.substring(9, 11));
    int liveCam = hexToByte(resp.substring(11, 13));
    
    if (SERIAL_DEBUG) {
        Serial.print("Preview camera: ");
        Serial.println(previewCam);
        Serial.print("Live camera: ");
        Serial.println(liveCam);
    }
    
    // Update LED indicators
    updateLEDs(previewCam, liveCam);
  }
}

int hexToByte(String hex) {
  return (int) strtol(hex.c_str(), NULL, 16);
}

void updateLEDs(int previewCam, int liveCam) {
  // CAMERA_ID is 1-based, but camera numbers from the switcher are 0-based.
  int camIndex = CAMERA_ID - 1;

  if (liveCam == camIndex) {
    // This ESP32's camera is live
    analogWrite(LED_PIN, LED_BRIGHTNESS_LIVE);
  } else if (previewCam == camIndex) {
    // This ESP32's camera is in preview
    analogWrite(LED_PIN, LED_BRIGHTNESS_PREVIEW);
  } else {
    // This ESP32's camera is not active
    analogWrite(LED_PIN, LED_BRIGHTNESS_OFF);
  }
}