#include "led.h"

#define LED_PIN 27

Led::Led(uint8_t brightness)
{
  FastLED.addLeds<WS2812, LED_PIN>(&_led, 1);
  FastLED.setBrightness(brightness);
}

Led::~Led() {}

void Led::drawpix(CRGB c)
{
  _led = _fixRgbOrder(c);
  FastLED.show();
}

void Led::drawpix(CHSV c)
{
  _led = c;
  FastLED.show();
}

void Led::fadeToBlackBy(uint8_t scale)
{
  _led.fadeToBlackBy(scale);
  FastLED.show();
}

CRGB Led::_fixRgbOrder(CRGB c)
{
  return CRGB(c.g, c.r, c.b);
}
