// TaskScheduler.cpp
#include "TaskScheduler.h"
#include "ScheduleManager/ScheduleManager.h"
#include "LEDController/LEDController.h"
#include <time.h>

extern LEDController ledController;

void handleScheduledLighting() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 1000) return;
    lastCheck = millis();

    String greenTime = ScheduleManager::getScheduledTime("green");
    String redTime = ScheduleManager::getScheduledTime("red");

    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    char currentTime[6];
    sprintf(currentTime, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    String nowStr(currentTime);

    if (nowStr == greenTime) {
        ledController.setColor("green");
    } else if (nowStr == redTime) {
        ledController.setColor("red");
    }
}
