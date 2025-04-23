#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "TaskScheduler/TaskScheduler.h"
#include "NetworkManager/NetworkManager.h"

LEDController ledController;
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC, &ledController);
WebServerManager webServerManager(&ledController, &mqttManager);
CaptivePortalManager captivePortal(WIFI_SSID, LOCAL_IP, GATEWAY_IP, REDIRECT_URL);

void setup() {
    Serial.begin(115200);
    NetworkManager::setupWiFiAndServices();
}

void loop() {
    handleScheduledLighting();
    NetworkManager::handleWiFiTasks();
    delay(10);
}
