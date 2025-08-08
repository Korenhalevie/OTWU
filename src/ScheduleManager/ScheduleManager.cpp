// ScheduleManager.cpp
#include "ScheduleManager.h"
#include <Preferences.h>

String ScheduleManager::getScheduledTime(const String& key) {
    Preferences pref;
    pref.begin("schedule", true);
    String value = pref.getString(key.c_str(), key == "green" ? "07:00" : "21:00");
    pref.end();
    return value;
}

void ScheduleManager::saveScheduledTime(const String& key, const String& time) {
    Preferences pref;
    pref.begin("schedule", false);
    pref.putString(key.c_str(), time);
    pref.end();
}

String ScheduleManager::getGreenWindows() {
    Preferences pref;
    pref.begin("schedule", true);
    String windows = pref.getString("greenWindows", "");
    pref.end();
    return windows;
}

void ScheduleManager::saveGreenWindows(const String& windows) {
    Preferences pref;
    pref.begin("schedule", false);
    pref.putString("greenWindows", windows);
    pref.end();
}

void ScheduleManager::clearGreenWindows() {
    Preferences pref;
    pref.begin("schedule", false);
    pref.putString("greenWindows", "");
    pref.end();
}
