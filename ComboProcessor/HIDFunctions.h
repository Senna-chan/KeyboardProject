// HIDFunctions.h

#ifndef _HIDFUNCTIONS_h
#define _HIDFUNCTIONS_h

#include "Arduino.h"

void PressMouse(uint8_t button);
void ReleaseMouse(uint8_t button);
void MoveMouse(byte x, byte y);
void moveScrollWheel(byte x, byte y);
void pressKeyboardKey(uint8_t key);
void printKeyboardString(String string);
void releaseKeyboardKey(uint8_t key);
void releaseAllKeyboardKeys();
void writeKeyboardKey(uint8_t key);
void pressConsumerKey(uint16_t key);
void releaseConsumerKey();
void writeConsumerKey(uint16_t key);

#endif

