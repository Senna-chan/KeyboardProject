#pragma once
template <typename T> unsigned int Serial_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial.read();
	return i;
}template <typename T> unsigned int Serial1_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial1.read();
	return i;
}template <typename T> unsigned int Serial2_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial2.read();
	return i;
}