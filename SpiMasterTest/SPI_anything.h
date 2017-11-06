#pragma once
#include <Arduino.h>
#include <Wire.h>

template <typename T> unsigned int I2C_writeAnything(const T& value)
{
	Wire.write((byte *)&value, sizeof(value));
	return sizeof(value);
}  // end of I2C_writeAnything
