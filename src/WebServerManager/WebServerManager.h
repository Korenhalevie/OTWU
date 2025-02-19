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

public:
    WebServerManager(LEDController* led, MQTTManager* mqtt);
    void setup();
};

#endif // WEBSERVER_MANAGER_H