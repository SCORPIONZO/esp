# ESP32 Web Server

This project creates a simple web server on the ESP32 microcontroller. It sets up the ESP32 as a WiFi access point and serves a basic web page with LED control functionality.

## Features

- Creates a WiFi access point with SSID "ESP32-WebServer"
- Serves a basic HTML page at the root path "/"
- Provides JSON status information at "/status"
- Shows uptime and system information
- Controls built-in LED via web interface (buttons to turn LED on/off/toggle)
- Shows current LED state on the main page

## LED Control Endpoints

- `/ledon` - Turn the LED on
- `/ledoff` - Turn the LED off
- `/ledtoggle` - Toggle the LED state
- The LED state is also shown in the status JSON at `/status`

## How to Use

1. Connect to the WiFi network "ESP32-WebServer" with password "123456789"
2. Open a web browser and navigate to `http://192.168.4.1`
3. The main page will display a welcome message and controls for the built-in LED
4. Use the buttons to turn the LED on, off, or toggle its state
5. Visit `http://192.168.4.1/status` to see JSON-formatted system information including LED state

## Building and Flashing

To build and flash this project:

```bash
idf.py set-target esp32
idf.py build
idf.py -p PORT flash
```

Replace PORT with your actual serial port (e.g., COM3 on Windows).

## Requirements

- ESP-IDF v4.4 or later
- ESP32 board
- Computer with WiFi to connect to the ESP32 AP
- Built-in LED typically connected to GPIO 2 (configurable via CONFIG_BLINK_GPIO)