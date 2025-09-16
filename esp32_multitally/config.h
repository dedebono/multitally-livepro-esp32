// Configuration file for ESP32 Multitally
// Update these values according to your setup

#ifndef CONFIG_H
#define CONFIG_H
#define MY_CAMERA_INDEX 1

// WiFi Configuration
const char* WIFI_SSID = "camera_server";
const char* WIFI_PASSWORD = "Ferarinomor123";

// Feelworld Device Configuration
const char* FEELWORLD_IP = "192.168.1.121";  // Replace with your Feelworld device IP
const int FEELWORLD_PORT = 1000;

// Hardware Configuration - Update GPIO pins based on your wiring
// Hardware Configuration - Update GPIO pins based on your wiring
const int LED_PIN = 2; // GPIO pin for the LED
const int NUM_LEDS = 1; // Number of WS2812B LEDs
const int CAMERA_ID = 1; // 1-4, set for each ESP32

// LED Behavior Configuration
const int LED_BRIGHTNESS_LIVE = 255;    // Full brightness for live camera
const int LED_BRIGHTNESS_PREVIEW = 128; // Half brightness for preview camera
const int LED_BRIGHTNESS_OFF = 0;       // Off for other cameras

// Network Settings
const unsigned long CONNECTION_TIMEOUT = 3000;  // 3 seconds
const unsigned long STATUS_UPDATE_INTERVAL = 1000;  // 1 second between status updates

// Debug Settings
const bool SERIAL_DEBUG = true;  // Set to false to disable serial debug output

#endif
