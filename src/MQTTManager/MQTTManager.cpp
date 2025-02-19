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

    while (!_client.connected())
    {
        String clientId = "ESP32_Client_";
        clientId += WiFi.macAddress();
        Serial.printf("Connecting to MQTT as %s...\n", clientId.c_str());

        if (_client.connect(clientId.c_str(), _username, _password))
        {
            Serial.println("Connected to MQTT!");
            _client.subscribe(_topic);
            _connected = true;
        }
        else
        {
            Serial.print("MQTT connection failed, error code: ");
            Serial.println(_client.state());
            Serial.println("Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void MQTTManager::loop()
{
    if (!_client.connected())
    {
        static unsigned long lastReconnectAttempt = 0;
        if (millis() - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = millis();
            Serial.println("Trying to reconnect to MQTT...");
            setup();
        }
    }
    else
    {
        _client.loop();
    }

    static unsigned long lastPing = 0;
    if (_client.connected() && millis() - lastPing > 10000)
    {
        _client.publish(_topic, "ping");
        lastPing = millis();
    }
}

void MQTTManager::publishMessage(const char* message)
{
    Serial.print("Publishing message: ");
    Serial.println(message);

    if (_client.publish(_topic, message))
    {
        Serial.println("Message published successfully");
    }
    else
    {
        Serial.println("Failed to publish message");
    }
}

void MQTTManager::publishColor(const char* color)
{
    Serial.print("Publishing color: ");
    Serial.println(color);
    publishMessage(color);
}

bool MQTTManager::isConnected()
{
    return _client.connected();
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Received MQTT message on topic: ");
    Serial.println(topic);

    String receivedMessage;
    for (unsigned int i = 0; i < length; i++) {
        receivedMessage += static_cast<char>(payload[i]);
    }
    receivedMessage.trim();

    Serial.println("Message: " + receivedMessage);

    if (_instance && _instance->_ledController)  
    {
        if (receivedMessage == "red") {
            _instance->_ledController->setColor("red");
        } 
        else if (receivedMessage == "green") {
            _instance->_ledController->setColor("green");
        } 
        else if (receivedMessage == "blue") {
            _instance->_ledController->setColor("blue");
        } 
        else if (receivedMessage == "off") {
            _instance->_ledController->setColor("off");
        } 
        else {
            Serial.println("Unknown color received!");
        }
    }
}
