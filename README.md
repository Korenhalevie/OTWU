# ESP32 LED MQTT Controller

## Overview
This project is an ESP32-based LED controller that integrates with an MQTT broker and a web server for remote control.

## Features
- Control LED color via MQTT messages
- Web interface for easy color selection
- WiFi connectivity
- Supports LED strip using FastLED library

## Hardware Requirements
- ESP32 development board
- WS2812 LED (or similar)
- WiFi network

## Software Requirements
- Arduino IDE / PlatformIO
- ESP32 board package
- Required libraries:
  - FastLED
  - PubSubClient
  - ESPAsyncWebServer
  - WiFi

## Installation
1. Clone the repository:
   ```sh
   git clone https://github.com/YOUR_USERNAME/ESP32-LED-MQTT.git
   cd ESP32-LED-MQTT
Open the project in your preferred development environment.
Install the required libraries.
Flash the code to your ESP32.
Configuration
Update the main.cpp file with your WiFi credentials and MQTT broker details:

cpp
Copy
Edit
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_topic = "ok_to_wake/color";
Usage
Connect your ESP32 to power.
It will automatically connect to WiFi and the MQTT broker.
Send MQTT messages to control the LED color.
Open the web interface to manually change the LED color.
MQTT Commands
Send messages to the MQTT topic to change LED color:

vbnet
Copy
Edit
Topic: ok_to_wake/color
Messages: "red", "green", "blue", "off"
Web Interface
Access the web interface via the ESP32's IP address in a browser. It provides buttons to change the LED color easily.

License
This project is open-source under the MIT License.

Author
Developed by Koren Halevie