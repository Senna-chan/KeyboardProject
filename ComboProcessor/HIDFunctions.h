// HIDFunctions.h

#ifndef _HIDFUNCTIONS_h
#define _HIDFUNCTIONS_h

#include "Arduino.h"
#include "Variables.h"

void releaseAllKeyboardMouse();

void pressMouse(uint8_t button);
void releaseMouse(uint8_t button);
void releaseMouseAll();
void moveMouse(byte x, byte y, byte wheelx, byte wheely);
void moveMouse(byte x, byte y);
void moveScrollWheel(byte x, byte y);
void pressKeyboardKey(uint8_t key);
void printKeyboardString(String string);
void releaseKeyboardKey(uint8_t key);
void releaseAllKeyboardKeys();
void writeKeyboardKey(uint8_t key);
void pressConsumerKey(uint16_t key);
void releaseConsumerKey();
void writeConsumerKey(uint16_t key);
void writeKeyReport(keyReport report);


#endif

