// ScheduleManager.h
#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <Arduino.h>

class ScheduleManager {
public:
    static String getScheduledTime(const String& key);
    static void saveScheduledTime(const String& key, const String& time);
    static String getGreenWindows();
    static void saveGreenWindows(const String& windows);
    static void clearGreenWindows();
};

#endif // SCHEDULE_MANAGER_H