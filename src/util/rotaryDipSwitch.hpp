/**
 * @file rotaryDipSwitch.hpp
 * @brief ロータリーDIPスイッチ用クラスヘッダ
 */

#ifndef ROTARY_DIP_SWITCH_HPP
#define ROTARY_DIP_SWITCH_HPP

#include <array>
#include <Arduino.h>

/**
 * @class RotaryDipSwitch
 * @brief ロータリーDIPスイッチ用クラス
 */

class RotaryDipSwitch {
private:
  const std::array<uint8_t, 4> _pins{};
  uint8_t _xor_mask = 0;

public:
  //! 引数のpinsは{1桁目のピン, 2桁目のピン, 3桁目のピン, 4桁目のピン}という風に指定する
  RotaryDipSwitch(std::array<uint8_t, 4> pins, bool is_pull_up = false):_pins(pins){
    for(uint8_t pin : pins){
      pinMode(pin, INPUT);
    }
    if(is_pull_up) {
      _xor_mask = 0b00001111;
    }
  }
  uint8_t read(void) const{
    uint8_t data = static_cast<uint8_t>(digitalRead(_pins[0]) | (digitalRead(_pins[1]) << 1) | 
              (digitalRead(_pins[2]) << 2) | (digitalRead(_pins[3]) << 3));
    return data ^ _xor_mask;
  }
};

#endif