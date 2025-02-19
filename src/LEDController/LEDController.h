#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>

#define LED_PIN      23
#define NUM_LEDS     1
#define BRIGHTNESS   255
#define LED_TYPE     WS2812
#define COLOR_ORDER  GRB

class LEDController {
private:
    CRGB _leds[NUM_LEDS];
    String _currentColor; 

public:
    LEDController();
    void setup();
    void setColor(const String& color);
    String getColor() const;
};

#endif // LED_CONTROLLER_H
