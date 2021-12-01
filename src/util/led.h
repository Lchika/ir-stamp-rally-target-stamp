#ifndef _X_LED_HPP_
#define _X_LED_HPP_

#include <FastLED.h>

class Led
{
public:
  Led(uint8_t brightness = 20);
  ~Led();
  void drawpix(CRGB c);
  void drawpix(CHSV c);
  void fadeToBlackBy(uint8_t scale);
  CRGB _led;

private:
  CRGB _fixRgbOrder(CRGB c);
};

#endif