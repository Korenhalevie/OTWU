// LEDController.h
#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>
#include <Preferences.h>

#define LED_PIN      23
#define NUM_LEDS     1
#define BRIGHTNESS   255
#define LED_TYPE     WS2812
#define COLOR_ORDER  GRB

class LEDController {
private:
    CRGB _leds[NUM_LEDS];
    String _currentColor;
    uint8_t _brightness; // 0-255
    Preferences _prefs;

public:
    LEDController();
    void setup();
    void setColor(const String& color);
    String getColor() const;
    void setBrightnessPercent(uint8_t percent); // 0-100
    uint8_t getBrightnessPercent() const;       // 0-100
};

#endif // LED_CONTROLLER_H
