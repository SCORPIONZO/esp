# ESP32 Web Server

This project creates a simple web server on the ESP32 microcontroller. It sets up the ESP32 as a WiFi access point and serves a basic web page.

## Features

- Creates a WiFi access point with SSID "ESP32-WebServer"
- Serves a basic HTML page at the root path "/"
- Provides JSON status information at "/status"
- Shows uptime and system information

## How to Use

1. Connect to the WiFi network "ESP32-WebServer" with password "123456789"
2. Open a web browser and navigate to `http://192.168.4.1`
3. The main page will display a welcome message and current uptime
4. Visit `http://192.168.4.1/status` to see JSON-formatted system information

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