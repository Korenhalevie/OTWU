// LEDController.cpp
#include "LEDController.h"

#define PREF_NAMESPACE "led"
#define PREF_KEY_BRIGHTNESS "brightness" // stored as uint8 0-100

LEDController::LEDController() : _currentColor("off"), _brightness(255) {}

void LEDController::setup()
{
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    // Load brightness from preferences (percent 0-100)
    _prefs.begin(PREF_NAMESPACE, false);
    uint8_t stored = _prefs.getUChar(PREF_KEY_BRIGHTNESS, 100); // default 100%
    if (stored > 100) stored = 100;
    setBrightnessPercent(stored);
    setColor("off");
}

void LEDController::setColor(const String& color)
{
    if (color == "green") {
        _leds[0] = CRGB::Green;
    } else if (color == "red") {
        _leds[0] = CRGB::Red;
    } else if (color == "blue") {
        _leds[0] = CRGB::Blue;
    } else {
        _leds[0] = CRGB::Black;
    }

    FastLED.setBrightness(_brightness); // ensure brightness applied
    FastLED.show();
    delay(10);
    _currentColor = color;
}

String LEDController::getColor() const
{
    return _currentColor;
}

void LEDController::setBrightnessPercent(uint8_t percent) {
    if (percent > 100) percent = 100;
    // Map 0-100% to 0-255 (linear)
    uint8_t newBrightness = (uint16_t)percent * 255 / 100;
    _brightness = newBrightness;
    FastLED.setBrightness(_brightness);
    FastLED.show();
    // persist percent (not 0-255 value)
    _prefs.putUChar(PREF_KEY_BRIGHTNESS, percent);
}

uint8_t LEDController::getBrightnessPercent() const {
    // Reverse map for UI (approx) _brightness 0-255 to 0-100
    return (uint16_t)_brightness * 100 / 255;
}
