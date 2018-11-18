// MacroHelper.h

#ifndef _MACROHELPER_h
#define _MACROHELPER_h

#include <Arduino.h>
#include "Variables.h"
#include <SdFs.h>
#include <i2cEncoderLibV2.h>
#include "Helpers.h"

void loadMacros();
void saveMacro();
void genMacro(uint8_t macroPos);
// Gets the keys in a hold blocking way from the keyboard
keyReport getMacroKeysFromKeyboard();

#endif