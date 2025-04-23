// MQTTManager.cpp
#include "MQTTManager.h"

MQTTManager* MQTTManager::_instance = nullptr;

MQTTManager::MQTTManager(const char* broker, int port, const char* topic, LEDController* ledController,
                         const char* username, const char* password)
    : _client(_espClient), _broker(broker), _port(port), _topic(topic), 
      _username(username), _password(password), _ledController(ledController), _connected(false)
{
    _instance = this;
}

void MQTTManager::setup()
{
    _client.setServer(_broker, _port);
    _client.setCallback(callback);

    connect();
}

void MQTTManager::connect()
{
    while (!_client.connected()) {
        String clientId = "ESP32_Client_" + WiFi.macAddress();
        Serial.printf("Connecting to MQTT as %s...\n", clientId.c_str());

        if (_client.connect(clientId.c_str(), _username, _password)) {
            Serial.println("Connected to MQTT!");
            _client.subscribe(_topic);
            _connected = true;
        } else {
            Serial.printf("MQTT connection failed, error code: %d\n", _client.state());
            Serial.println("Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void MQTTManager::loop()
{
    if (!_client.connected()) {
        static unsigned long lastReconnectAttempt = 0;
        if (millis() - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = millis();
            Serial.println("Trying to reconnect to MQTT...");
            connect();
        }
    } else {
        _client.loop();
    }

    static unsigned long lastPing = 0;
    if (_client.connected() && millis() - lastPing > 10000) {
        _client.publish(_topic, "ping");
        lastPing = millis();
    }
}

void MQTTManager::publishMessage(const char* message)
{
    Serial.printf("Publishing message: %s\n", message);
    if (_client.publish(_topic, message)) {
        Serial.println("Message published successfully");
    } else {
        Serial.println("Failed to publish message");
    }
}

void MQTTManager::publishColor(const char* color)
{
    publishMessage(color);
}

bool MQTTManager::isConnected()
{
    return _client.connected();
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length)
{
    String receivedMessage;
    for (unsigned int i = 0; i < length; i++) {
        receivedMessage += static_cast<char>(payload[i]);
    }
    receivedMessage.trim();

    if (receivedMessage == "ping") {
        return;
    }
    
    Serial.printf("MQTT message on %s: %s\n", topic, receivedMessage.c_str());

    if (_instance && _instance->_ledController) {
        _instance->_ledController->setColor(receivedMessage);
    }
}