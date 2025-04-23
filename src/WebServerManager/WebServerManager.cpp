// WebServerManager.cpp 
#include "WebServerManager.h"
#include <Preferences.h>
#include "ScheduleManager/ScheduleManager.h"

WebServerManager::WebServerManager(LEDController* led, MQTTManager* mqtt)
    : _server(80), _ledController(led), _mqttManager(mqtt) {}

void WebServerManager::setup() {
    setupRootPage();
    setupColorHandler();
    setupScheduleHandler();
    setupClearScheduleHandler();
    setupForgetWiFiHandler();
    _server.begin();
}

void WebServerManager::setupRootPage() {
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String greenTime = ScheduleManager::getScheduledTime("green");
        String redTime = ScheduleManager::getScheduledTime("red");
        request->send(200, "text/html", generateHtmlPage(greenTime, redTime));
    });
}

void WebServerManager::setupColorHandler() {
    _server.on("/setColor", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (request->hasParam("color")) {
            String color = request->getParam("color")->value();
            _ledController->setColor(color);
            _mqttManager->publishColor(color.c_str());
            request->send(200, "text/plain", "Color changed to " + color);
        } else {
            request->send(400, "text/plain", "Missing color parameter");
        }
    });
}

void WebServerManager::setupScheduleHandler() {
    _server.on("/setSchedule", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasParam("greenTime") && request->hasParam("redTime")) {
            ScheduleManager::saveScheduledTime("green", request->getParam("greenTime")->value());
            ScheduleManager::saveScheduledTime("red", request->getParam("redTime")->value());
            request->send(200, "text/plain", "Schedule saved!");
        } else {
            request->send(400, "text/plain", "Missing time values");
        }
    });
}


void WebServerManager::setupClearScheduleHandler() {
    _server.on("/clearSchedule", HTTP_GET, [](AsyncWebServerRequest* request) {
        ScheduleManager::saveScheduledTime("green", "");
        ScheduleManager::saveScheduledTime("red", "");
        request->send(200, "text/plain", "Schedule cleared!");
    });
}

void WebServerManager::setupForgetWiFiHandler() {
    _server.on("/forgetWiFi", HTTP_GET, [](AsyncWebServerRequest* request) {
        Preferences pref;
        pref.begin("wifi", false);
        pref.clear();
        pref.end();

        WiFi.disconnect(true, true);
        request->send(200, "text/plain", "WiFi cleared. Restarting...");
        delay(1000);
        ESP.restart();
    });
}

String WebServerManager::generateHtmlPage(const String& greenTime, const String& redTime) {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
        <title>LED Control</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {
                font-family: Arial, sans-serif;
                text-align: center;
                background-color: #f5f5f5;
                margin: 0;
                padding: 0;
            }
            h1 {
                margin-top: 20px;
                color: #333;
            }
            #clock {
                font-size: 3em;
                font-weight: bold;
                margin: 20px 0;
                color: #2c3e50;
            }
            button {
                padding: 12px 24px;
                margin: 10px;
                font-size: 1.2em;
                border: none;
                border-radius: 8px;
                cursor: pointer;
                background-color: #3498db;
                color: white;
                transition: background-color 0.3s ease;
            }
            button:hover {
                background-color: #2980b9;
            }
            .card {
                background-color: #fff;
                border-radius: 10px;
                padding: 20px;
                max-width: 400px;
                margin: 30px auto;
                box-shadow: 0 0 10px rgba(0,0,0,0.1);
            }
            .input-group {
                margin: 10px 0;
                text-align: left;
            }
            label {
                display: block;
                margin-bottom: 5px;
                font-weight: bold;
            }
            input[type="time"] {
                padding: 10px;
                font-size: 1em;
                width: 100%;
                box-sizing: border-box;
            }
            input[type="submit"], .primary-btn, .danger-btn {
                margin-top: 10px;
                padding: 10px 20px;
                font-size: 1em;
                border: none;
                border-radius: 6px;
                cursor: pointer;
            }
            .primary-btn {
                background-color: #2ecc71;
                color: white;
            }
            .primary-btn:hover {
                background-color: #27ae60;
            }
            .danger-btn {
                background-color: #e74c3c;
                color: white;
            }
            .danger-btn:hover {
                background-color: #c0392b;
            }
            #saveStatus {
                transition: opacity 0.3s ease;
            }
        </style>
        <script>
            function updateTime() {
                var now = new Date();
                document.getElementById('clock').innerText = now.toLocaleTimeString();
            }
            setInterval(updateTime, 1000);
            window.onload = updateTime;

            function changeColor(color) {
                fetch('/setColor?color=' + color)
                    .then(() => {
                        document.getElementById('currentColor').innerText = color;
                    });
            }

            function showStatus(message, isSuccess) {
                const status = document.getElementById("saveStatus");
                status.innerText = message;
                status.style.color = isSuccess ? "green" : "red";
                status.style.opacity = "1";
                setTimeout(() => {
                    status.style.opacity = "0";
                }, 3000);
            }

            function forgetWiFi() {
                Swal.fire({
                    title: 'Forget WiFi?',
                    text: "Are you sure you want to disconnect and remove saved WiFi credentials?",
                    icon: 'warning',
                    showCancelButton: true,
                    confirmButtonColor: '#e74c3c',
                    cancelButtonColor: '#3085d6',
                    confirmButtonText: 'Yes, forget it',
                    cancelButtonText: 'Cancel'
                }).then((result) => {
                    if (result.isConfirmed) {
                        Swal.fire({
                            title: 'Forgetting WiFi...',
                            text: 'Please wait while the device restarts',
                            icon: 'info',
                            showConfirmButton: false,
                            allowOutsideClick: false
                        });
                        fetch("/forgetWiFi")
                            .then(() => {
                                Swal.fire({
                                    title: '✅ WiFi Cleared!',
                                    text: 'Device is restarting...',
                                    icon: 'success',
                                    showConfirmButton: false
                                });
                            })
                            .catch(() => {
                                Swal.fire('Error', 'Failed to forget WiFi.', 'error');
                            });
                    }
                });
            }

            document.addEventListener("DOMContentLoaded", () => {
                const form = document.getElementById("scheduleForm");
                form.addEventListener("submit", function(e) {
                    e.preventDefault();
                    const green = document.getElementById("greenTime").value;
                    const red = document.getElementById("redTime").value;
                    showStatus("Saving...", true);
                    fetch("/setSchedule?greenTime=" + green + "&redTime=" + red)
                        .then(res => res.text())
                        .then(text => showStatus("✔ " + text, true))
                        .catch(() => showStatus("❌ Failed to save", false));
                });
            });

            function clearSchedule() {
                showStatus("Clearing...", true);

                fetch("/clearSchedule")
                    .then(res => res.text())
                    .then(text => {
                        document.getElementById("greenTime").value = "";
                        document.getElementById("redTime").value = "";
                        showStatus("✔ " + text, true);
                    })
                    .catch(() => showStatus("❌ Failed to clear", false));
            }

        </script>
    </head>
    <body>
        <h1>OTWU Control Panel</h1>
        <div id="clock">--:--:--</div>
        <div>
            <button onclick="changeColor('red')" style="background-color:#e74c3c;">Red</button>
            <button onclick="changeColor('green')" style="background-color:#2ecc71;">Green</button>
            <button onclick="changeColor('blue')" style="background-color:#3498db;">Blue</button>
            <button onclick="changeColor('off')" style="background-color:#7f8c8d;">Off</button>
        </div>
        <div style="margin:20px auto; font-size:1.5em; font-weight:bold; color:#555;">
            Current Color: <span id="currentColor">)rawliteral" + _ledController->getColor() + R"rawliteral(</span>
        </div>
        <div class="card">
            <form id="scheduleForm">
                <h2>Set LED Schedule</h2>
                <div class="input-group">
                    <label>Green Time:</label>
                    <input type="time" id="greenTime" value=")rawliteral" + greenTime + R"rawliteral(" required>
                </div>
                <div class="input-group">
                    <label>Red Time:</label>
                    <input type="time" id="redTime" value=")rawliteral" + redTime + R"rawliteral(" required>
                </div>
                <input type="submit" value="Save Schedule" class="primary-btn">
                <button type="button" onclick="clearSchedule()" class="danger-btn" style="margin-top:10px;">Clear Schedule</button>
                <div id="saveStatus" style="margin-top: 10px; font-weight: bold;"></div>
                <hr style="margin-top:20px;">
                <button type="button" onclick="forgetWiFi()" class="danger-btn" style="background-color:#aaa;">Forget WiFi</button>
            </form>
        </div>
    </body>
    </html>
    )rawliteral";

    return html;
}
