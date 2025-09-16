# ESP32 Multitally for Feelworld Devices

A hardware implementation of the multitally system using ESP32 to communicate with Feelworld video switchers and display camera status using LEDs.

## Features

- Connects to Feelworld video switchers via UDP protocol
- Displays connection and camera status using WS2812B RGB LED indicators
- Monitors specified camera (default Camera 1) for live/preview status
- Automatic reconnection on network issues
- Configurable via defines in the .ino file

## Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 recommended)
- WS2812B LED strip or individual WS2812B LED (1 LED for single camera indicator)
- Breadboard and jumper wires
- USB cable for programming

## Wiring Diagram

```
ESP32 GPIO2 (LED_PIN) → WS2812B Data In
WS2812B VCC → 5V or 3.3V (check your WS2812B spec)
WS2812B GND → GND
```

**Note**: The WS2812B LED will display connection and camera status (monitors specified camera, default 1):
- Purple: Connected to WiFi
- Blue: Connected to Feelworld device (no status received yet)
- Red: Specified camera is live
- Green: Specified camera is in preview
- Off: Specified camera is neither live nor preview

## Setup Instructions

### 1. Install Arduino IDE and ESP32 Support

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - File → Preferences → Additional Boards Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search for "ESP32" and install

### 2. Install Required Libraries

Install these libraries via Arduino Library Manager (Sketch → Include Library → Manage Libraries):
- WiFi (usually included with ESP32)
- WiFiUDP (usually included with ESP32)
- Adafruit NeoPixel (by Adafruit)

### 3. Configure the Project

1. Open `esp32_multitally.ino` in Arduino IDE
2. Update the `#define` statements at the top of the file with your settings:
   - `WIFI_SSID`: Your WiFi network name
   - `WIFI_PASSWORD`: Your WiFi password
   - `FEELWORLD_IP`: IP address of your Feelworld device
   - `LED_PIN`: GPIO pin for WS2812B data (default 2)
   - `NUM_LEDS`: Number of WS2812B LEDs (default 1)
   - `MY_CAMERA_INDEX`: Camera ID to monitor (1-4, default 1)

### 4. Upload to ESP32

1. Connect ESP32 via USB
2. Select board: Tools → Board → ESP32 Dev Module
3. Select correct COM port
4. Click Upload button

## Usage

1. Power on the ESP32
2. The device will automatically:
   - Connect to WiFi
   - Connect to Feelworld device
   - Start polling for camera status every second
   - Update LED indicators accordingly

## LED Status Indicators

- **Purple**: Connected to WiFi
- **Blue**: Connected to Feelworld device (no status received yet)
- **Red**: Specified camera is live
- **Green**: Specified camera is in preview
- **Off**: Specified camera is neither live nor preview

## Code Overview

The main code is in `esp32_multitally.ino` with configuration via `#define` statements at the top of the file.

### Key Functions

- `setup()`: Initializes serial, NeoPixel strip, connects to WiFi and Feelworld.
- `loop()`: Polls for camera status if connected, otherwise attempts reconnection.
- `connectWiFi()`: Connects to WiFi and updates LED.
- `feelworldConnect()`: Sends connection command to Feelworld device and updates LED.
- `pollStatus()`: Requests camera status from Feelworld device and parses response.
- `updateLEDDecision()`: Sets the LED color based on connection and camera status.

## Protocol Details

The ESP32 communicates with Feelworld devices using a custom UDP protocol:

### Connection Command
```
<T00010068006600000000>
```

### Status Request Command  
```
<T000100F14001000000>
```

### Response Format
Responses follow the pattern: `<Fxxxxxxxxxx>` where specific bytes indicate camera states.

## Troubleshooting

### Common Issues

1. **WiFi Connection Fails**
   - Check SSID and password in the #define statements in esp32_multitally.ino
   - Ensure ESP32 is in range of WiFi network

2. **Cannot Connect to Feelworld Device**
   - Verify Feelworld device IP address in the #define statements
   - Check that Feelworld device is on same network
   - Ensure Feelworld Open API is enabled

3. **WS2812B LED Not Lighting**
   - Check wiring connections (Data In to GPIO2, VCC to power, GND to GND)
   - Verify LED_PIN in the #define statements matches your wiring
   - Ensure WS2812B is powered correctly (5V or 3.3V as per spec)
   - Check if Adafruit NeoPixel library is installed

### Serial Debug

Enable serial monitor (Tools → Serial Monitor) at 115200 baud to see debug messages:
- WiFi connection status
- Feelworld connection attempts
- Status requests and responses
- Error messages

## Customization

### Changing Monitored Camera
Update the `MY_CAMERA_INDEX` #define to monitor a different camera (1-4).

### Different LED Behaviors
Modify the `updateLEDDecision()` function to implement:
- Blinking patterns
- Different colors for different states (using WS2812B RGB capabilities)
- Brightness control via RGB values

### Web Interface
The code can be extended to include a web server for configuration via browser.

## Supported Devices

Tested with:
- Feelworld L1 Video Switcher
- Feelworld L2 Plus Video Switcher
- RGBlink mini

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:
- Bug fixes
- New features
- Additional device support
- Documentation improvements
