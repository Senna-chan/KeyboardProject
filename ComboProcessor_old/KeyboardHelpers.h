// KeyboardHelpers.h

#ifndef _KEYBOARDHELPERS_h
#define _KEYBOARDHELPERS_h

#include <Arduino.h>
#include "Variables.h"



char getAsciiFromKeyboard();
keyReport getKeysFromKeyboard();
char convertHIDToASCII(byte keyCode);
void waitNoKeysPressed();
static bool pressedShift;


#endif

