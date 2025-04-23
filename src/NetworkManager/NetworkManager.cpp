// NetworkManager.cpp
#include "NetworkManager.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h>
#include "./Config.h"
#include "CaptivePortalManager/CaptivePortalManager.h"
#include "LEDController/LEDController.h"
#include "MQTTManager/MQTTManager.h"
#include "WebServerManager/WebServerManager.h"
#include "Globals.h"

extern LEDController ledController;
extern MQTTManager mqttManager;
extern WebServerManager webServerManager;
extern CaptivePortalManager captivePortal;

namespace NetworkManager
{

    void setupWiFiAndServices()
    {

        bool connected = captivePortal.connectToWiFi();

        if (!connected)
        {
            Serial.println("⚠️ Not connected to WiFi. Running in Access Point mode.");
            return;
        }

        Serial.println("✅ Connected to WiFi!");

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
        {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\n🚀 Starting MQTT and Web Server...");

            if (MDNS.begin("otw"))
            {
                Serial.println("🌍 mDNS responder started at http://otw.local");
            }
            else
            {
                Serial.println("❌ Failed to start mDNS");
            }

            configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
            Serial.println("⏳ Waiting for time sync...");
            unsigned long start = millis();
            while (time(nullptr) < 100000 && millis() - start < 10000)
            {
                delay(500);
                Serial.print(".");
            }
            Serial.println("\n⏰ Time synced!");

            ledController.setup();
            mqttManager.setup();
            webServerManager.setup();
        }
        else
        {
            Serial.println("\n⚠️ No internet connection, skipping MQTT setup.");
        }
    }

    void handleWiFiTasks()
    {
        if (WiFi.status() == WL_CONNECTED && mqttManager.isConnected())
        {
            mqttManager.loop();
        }
        else
        {
            Serial.println("⚠️ MQTT not running - No WiFi connection.");
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            captivePortal.processDNSRequests();
        }
    }

} // namespace NetworkManager
