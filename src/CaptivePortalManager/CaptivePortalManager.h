#ifndef CAPTIVE_PORTAL_MANAGER_H
#define CAPTIVE_PORTAL_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <Preferences.h>

class CaptivePortalManager
{
private:
    DNSServer dnsServer;
    AsyncWebServer server;
    Preferences preferences;
    const char *ssid;
    IPAddress localIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    String redirectURL;
    bool connectedMode;

    void saveWiFiCredentials(const String &ssid, const String &password);
    bool loadWiFiCredentials(String &ssid, String &password);
    String generateWiFiSetupPage(int numNetworks);

public:
    CaptivePortalManager(const char *ssid, const IPAddress &localIP, const IPAddress &gatewayIP, const String &redirectURL);
    bool connectToWiFi(); 
    void start();  
    void processDNSRequests();
};

#endif // CAPTIVE_PORTAL_MANAGER_H
