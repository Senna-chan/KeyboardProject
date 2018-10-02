// OledFunctions.h

#ifndef _OLEDFUNCTIONS_h
#define _OLEDFUNCTIONS_h

#include "Config.h"

const String LOWER_ALPHABET		= "abcdefghijklmnopqrstuvwxyz";
const String UPPER_ALPHABET		= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const String NUMBERS			= "0123456789.-";
const String HEXCHARS			= "ABCDEF0123456789";

void clearNonHudArea();
void redrawhud();
void drawmenu();
void fullupdateoled();
void drawfunctype();
bool yesnoScreen(bool defaultval);
bool yesnoScreen(bool defaultval, char* line1, char* line2);
char getAsciiFromKeyboard();
char getKeysFromKeyboard(uint8_t *modifiers);
String inputScreen(uint8_t maxLength, char* line);
String inputScreen(uint8_t maxLength, char* line, String regex);
String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty);
String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty, String regex);


static bool pressedShift;

#endif

