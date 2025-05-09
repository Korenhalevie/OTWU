// WebServerManager.h
#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
#include "LEDController/LEDController.h"
#include "MQTTManager/MQTTManager.h"

class WebServerManager {
private:
    AsyncWebServer _server;
    LEDController* _ledController;
    MQTTManager* _mqttManager;

    void setupRootPage();
    void setupColorHandler();
    void setupScheduleHandler();
    void setupClearScheduleHandler();
    void setupForgetWiFiHandler();
    String generateHtmlPage(const String& greenTime, const String& redTime);

public:
    WebServerManager(LEDController* led, MQTTManager* mqtt);
    void setup();
};

#endif // WEBSERVERMANAGER_H
