#include "CaptivePortalManager.h"

CaptivePortalManager::CaptivePortalManager(const char *ssid, const IPAddress &localIP, const IPAddress &gatewayIP, const String &redirectURL)
    : server(80), ssid(ssid), localIP(localIP), gatewayIP(gatewayIP), subnetMask(255, 255, 255, 0), redirectURL(redirectURL)
{
    connectedMode = false;
}

void CaptivePortalManager::start()
{
    Serial.println("🚀 Starting Captive Portal...");

    WiFi.disconnect(true);
    delay(100);

    Serial.println("🔍 Scanning for WiFi networks...");
    int numNetworks = WiFi.scanNetworks();

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(ssid);

    dnsServer.setTTL(3600);
    dnsServer.start(53, "*", localIP);

    connectedMode = (WiFi.status() == WL_CONNECTED);

    // ROOT PAGE
    server.on("/", HTTP_GET, [this, numNetworks](AsyncWebServerRequest *request) {
        if (connectedMode) {
            String html = "<html><body><h1>You're connected!</h1></body></html>";
            request->send(200, "text/html", html);
        } else {
            String html = "<html><head><title>ESP32 Captive Portal</title></head><body>";
            html += "<h1>Choose WiFi Network</h1>";
            html += "<form action='/save' method='POST'>";
            html += "SSID: <select name='ssid'>";
            if (numNetworks == 0) {
                html += "<option value=''>No networks found</option>";
            } else {
                for (int i = 0; i < numNetworks; i++) {
                    html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
                }
            }
            html += "</select><br>";
            html += "Password: <input type='password' name='password'><br>";
            html += "<input type='submit' value='Connect'>";
            html += "</form></body></html>";
            request->send(200, "text/html", html);
        }
    });

    // SAVE WiFi CREDENTIALS
    server.on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            String ssid = request->getParam("ssid", true)->value();
            String password = request->getParam("password", true)->value();
            saveWiFiCredentials(ssid, password);
            request->send(200, "text/plain", "✅ WiFi credentials saved! Restarting...");
            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "❌ Missing SSID or Password");
        }
    });

    // CAPTIVE PORTAL DETECTION ROUTES
    server.on("/generate_204", [](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
      });
      

    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "OK");
    });

    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Success");
    });

    server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Success");
    });

    server.on("/captiveportal", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // CATCH-ALL: FORCE REDIRECT
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("http://4.3.2.1");
        Serial.print("onnotfound ");
        Serial.print(request->host());
        Serial.print(" ");
        Serial.print(request->url());
        Serial.println(" sent redirect to http://4.3.2.1");
      });
      

    server.begin();
    Serial.println("✅ Captive Portal Started at 192.168.4.1");
}

void CaptivePortalManager::processDNSRequests()
{
    dnsServer.processNextRequest();
}

void CaptivePortalManager::saveWiFiCredentials(const String &ssid, const String &password)
{
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
}

bool CaptivePortalManager::loadWiFiCredentials(String &ssid, String &password)
{
    preferences.begin("wifi", true);
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    preferences.end();
    return !ssid.isEmpty();
}

bool CaptivePortalManager::connectToWiFi()
{
    WiFi.mode(WIFI_STA);
    String ssid, password;

    if (loadWiFiCredentials(ssid, password)) {
        Serial.printf("📶 Trying to connect to WiFi: %s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ Connected to WiFi!");
            Serial.print("📡 IP Address: ");
            Serial.println(WiFi.localIP());
            return true;
        } else {
            Serial.println("\n❌ Failed to connect within timeout.");
        }
    } else {
        Serial.println("⚠️ No saved WiFi credentials found.");
    }

    Serial.println("🔄 Switching to AP mode and starting Captive Portal...");
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    start();
    return false;
}
