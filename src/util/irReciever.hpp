#ifndef IR_RECIEVER_HPP
#define IR_RECIEVER_HPP

#include <Arduino.h>
#include <functional>

struct IrReceiveConfig
{
  unsigned int data_t = 500;     // 通信プロトコルのT[us]
  int tolerance[2] = {100, 200}; // H信号の誤差裕度, L信号の誤差裕度[us]
};

class IrReceiver
{
public:
  IrReceiver(int receive_pin, IrReceiveConfig config) : receive_pin_(receive_pin),
                                                        config_(config)
  {
  }
  void setup(void (*update_func)(void))
  {
    update_func_ = update_func;
    attachInterrupt(digitalPinToInterrupt(receive_pin_), update_func_, CHANGE);
  }
  void update_data()
  {
    detachInterrupt(digitalPinToInterrupt(receive_pin_));

    int ir_state = digitalRead(receive_pin_);
    unsigned int signal_time = micros() - ir_time_;

    if (signal_time > config_.data_t - config_.tolerance[ir_state])
    {
      ir_time_ += signal_time;
      int ir_count = int((signal_time + config_.tolerance[ir_state]) / config_.data_t);
      if (ir_count == 3 && ir_state == HIGH)
      {
        ir_tmp_data_ = 0b111;
        received_time = millis();
      }
      else if (ir_count == 2 && ir_state == HIGH)
      {
        ir_tmp_data_ = (ir_tmp_data_ << 2) + 0b11;
        ir_received_data = ir_tmp_data_;
        ir_tmp_data_ = 0;
      }
      else
      {
        ir_tmp_data_ = ir_tmp_data_ << ir_count;
        if (ir_state == HIGH)
        {
          ir_tmp_data_ = ir_tmp_data_ + (1 << (ir_count - 1));
        }
      }
    }
    attachInterrupt(digitalPinToInterrupt(receive_pin_), update_func_, CHANGE);
  }
  volatile int ir_received_data = 0;
  volatile unsigned long received_time = 0;

private:
  int receive_pin_ = 0;
  IrReceiveConfig config_;
  volatile unsigned int ir_time_ = 0;
  volatile int ir_tmp_data_ = 0;
  void (*update_func_)(void);
};

#endif