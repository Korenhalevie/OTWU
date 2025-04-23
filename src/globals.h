#ifndef GLOBALS_H
#define GLOBALS_H

#include "LEDController/LEDController.h"
#include "MQTTManager/MQTTManager.h"
#include "WebServerManager/WebServerManager.h"
#include "CaptivePortalManager/CaptivePortalManager.h"

extern LEDController ledController;
extern MQTTManager mqttManager;
extern WebServerManager webServerManager;
extern CaptivePortalManager captivePortal;

#endif // GLOBALS_H
