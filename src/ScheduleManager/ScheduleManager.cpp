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
