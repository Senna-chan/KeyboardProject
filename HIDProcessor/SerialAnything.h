#pragma once
template <typename T> unsigned int Serial1_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++) {
		while (Serial1.available() == 0); /*WAIT */
		*p++ = Serial1.read();
	}
	return i;
}
template <typename T> unsigned int Serial_readAnything(T& value)
{
	byte * p = (byte*)&value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = Serial.read();
	return i;
}