// LEDController.cpp
#include "LEDController.h"

LEDController::LEDController() : _currentColor("off") {}

void LEDController::setup()
{
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
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

    FastLED.show();
    delay(10);
    _currentColor = color;
}

String LEDController::getColor() const
{
    return _currentColor;
}
