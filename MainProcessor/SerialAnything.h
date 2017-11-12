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

template <typename T> unsigned int Serial_writeAnything(const T& value)
{
	Serial.write((byte *)&value, sizeof(value));
	return sizeof(value);
}

template <typename T> unsigned int Serial1_writeAnything(const T& value)
{
	Serial1.write((byte *)&value, sizeof(value));
	return sizeof(value);
}

template <typename T> unsigned int Serial2_writeAnything(const T& value)
{
	Serial2.write((byte *)&value, sizeof(value));
	return sizeof(value);
}