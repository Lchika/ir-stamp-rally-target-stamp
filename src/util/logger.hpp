#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <Arduino.h>

class Logger
{
private:
	bool _is_enable = false;

public:
	void enable(unsigned long baurate = 115200)
	{
		Serial.begin(baurate);
		_is_enable = true;
	}
	template <class... Args>
	void printIfEnable(const char *format, Args const &...args)
	{
		if (_is_enable && Serial)
		{
			Serial.printf(format, args...);
		}
	}
};

#endif