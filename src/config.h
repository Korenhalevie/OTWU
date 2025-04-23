// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

// MQTT Configuration
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC "ok_to_wake/color"

// WiFi Configuration
#define WIFI_SSID "OTWU"

// Static IP Configuration
const IPAddress LOCAL_IP(4, 3, 2, 1);
const IPAddress GATEWAY_IP(4, 3, 2, 1);
const IPAddress SUBNET_MASK(255, 255, 255, 0);
const IPAddress DNS(8, 8, 8, 8);

// Redirect URL for Captive Portal
const String REDIRECT_URL = "http://192.168.1.200";

#endif // CONFIG_H
