#include <WiFi.h>
#include "LEDController/LEDController.h"
#include "MQTTManager/MQTTManager.h"
#include "WebServerManager/WebServerManager.h"

const char *SSID = "Halevie";
const char *PASSWORD = "koren123";
const char *MQTT_BROKER = "broker.emqx.io";
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "ok_to_wake/color";

LEDController ledController;
MQTTManager mqttManager(MQTT_BROKER, MQTT_PORT, MQTT_TOPIC, &ledController);
WebServerManager webServerManager(&ledController, &mqttManager);

/**
 * @brief establish a connection to the WiFi network
 */
void connectToWiFi()
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup()
{
    Serial.begin(115200);

    ledController.setup();
    connectToWiFi();
    mqttManager.setup();
    webServerManager.setup();
}

void loop()
{
    mqttManager.loop();
    delay(10);  
}
