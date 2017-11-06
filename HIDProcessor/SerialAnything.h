#pragma once
template <typename T> unsigned int Serial1_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial1.read();
	return i;
}  // end of I2C_readAnything
template <typename T> unsigned int Serial_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial.read();
	return i;
}  // end of I2C_readAnything