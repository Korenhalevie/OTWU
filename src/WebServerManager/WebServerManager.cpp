#include "WebServerManager.h"

WebServerManager::WebServerManager(LEDController* led, MQTTManager* mqtt)
    : _server(80), _ledController(led), _mqttManager(mqtt) {}

void WebServerManager::setup()
{
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request)
    {
        String html = "<html><head><title>LED Control</title></head><body>";
        html += "<h1>Control LED Color</h1>";
        html += "<button onclick=\"changeColor('red')\">Red</button>";
        html += "<button onclick=\"changeColor('green')\">Green</button>";
        html += "<button onclick=\"changeColor('blue')\">Blue</button>";
        html += "<button onclick=\"changeColor('off')\">Off</button>";
        html += "<script>function changeColor(color) {";
        html += "fetch('/setColor?color=' + color);";
        html += "}</script>";
        html += "</body></html>";

        request->send(200, "text/html", html);
    });

    _server.on("/setColor", HTTP_GET, [this](AsyncWebServerRequest* request)
    {
        if (request->hasParam("color")) 
        {
            String color = request->getParam("color")->value();
            _ledController->setColor(color);
            _mqttManager->publishColor(color.c_str());

            request->send(200, "text/plain", "Color changed to " + color);
        } 
        else 
        {
            request->send(400, "text/plain", "Missing color parameter");
        }
    });

    _server.begin();
}
