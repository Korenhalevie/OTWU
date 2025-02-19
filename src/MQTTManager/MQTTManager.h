#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "LEDController/LEDController.h"
#include <PubSubClient.h>
#include <WiFi.h>

class MQTTManager
{
private:
    static MQTTManager* _instance;

    LEDController* _ledController;  
    WiFiClient _espClient;       
    PubSubClient _client;         
    const char* _broker;       
    int _port;                      
    const char* _topic;      
    const char* _username;          
    const char* _password;         
    bool _connected;               

public:
    MQTTManager(const char* broker, int port, const char* topic, LEDController* ledController, 
                const char* username = nullptr, const char* password = nullptr);

    void setup();
    void loop();
    void publishMessage(const char* message);
    void publishColor(const char* color);
    bool isConnected();

    static void callback(char* topic, byte* payload, unsigned int length);
};

#endif // MQTT_MANAGER_H
