# ESP32 Multitally for Feelworld Devices

A hardware implementation of the multitally system using ESP32 to communicate with Feelworld video switchers and display camera status using LEDs.

## Features

- Connects to Feelworld video switchers via UDP protocol
- Displays camera status using LED indicators
- Supports up to 4 cameras (live, preview, offline states)
- Automatic reconnection on network issues
- Configurable via simple header file

## Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 recommended)
- 4x LEDs (different colors recommended for live/preview states)
- 4x Current-limiting resistors (220-330Ω)
- Breadboard and jumper wires
- USB cable for programming

## Wiring Diagram

```
ESP32 GPIO2  → LED1 + Resistor → GND
ESP32 GPIO4  → LED2 + Resistor → GND  
ESP32 GPIO5  → LED3 + Resistor → GND
ESP32 GPIO18 → LED4 + Resistor → GND
```

**Note**: Use different colored LEDs for better visual distinction:
- Red: Live camera
- Yellow/Orange: Preview camera
- Green: Online but not active
- Off: Camera offline

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
- ArduinoJson (by Benoit Blanchon)

### 3. Configure the Project

1. Open `esp32_multitally.ino` in Arduino IDE
2. Update `config.h` with your settings:
   - `WIFI_SSID`: Your WiFi network name
   - `WIFI_PASSWORD`: Your WiFi password  
   - `FEELWORLD_IP`: IP address of your Feelworld device
   - Update `CAMERA_LED_PINS` if using different GPIO pins

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

- **Solid Bright**: Camera is LIVE (on air)
- **Solid Dim**: Camera is in PREVIEW
- **Off**: Camera is offline or not connected

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
   - Check SSID and password in config.h
   - Ensure ESP32 is in range of WiFi network

2. **Cannot Connect to Feelworld Device**
   - Verify Feelworld device IP address
   - Check that Feelworld device is on same network
   - Ensure Feelworld Open API is enabled

3. **LEDs Not Lighting**
   - Check wiring connections
   - Verify GPIO pins in config.h match your wiring
   - Ensure resistors are properly connected

### Serial Debug

Enable serial monitor (Tools → Serial Monitor) at 115200 baud to see debug messages:
- WiFi connection status
- Feelworld connection attempts
- Status requests and responses
- Error messages

## Customization

### Adding More Cameras
Update the `CAMERA_LED_PINS` array in `config.h` and modify the parsing logic if your Feelworld device supports more than 4 cameras.

### Different LED Behaviors
Modify the `updateLEDs()` function to implement:
- Blinking patterns
- Different colors for different states
- PWM brightness control

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
