// OledFunctions.h

#ifndef _OLEDFUNCTIONS_h
#define _OLEDFUNCTIONS_h


#include <Arduino.h>
#include "Variables.h"

const String LOWER_ALPHABET		= "abcdefghijklmnopqrstuvwxyz";
const String UPPER_ALPHABET		= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const String NUMBERS			= "0123456789.-";
const String HEXCHARS			= "ABCDEF0123456789";
const String ALPHABET			= LOWER_ALPHABET + UPPER_ALPHABET;

void clearNonHudArea();
void redrawhud();
void drawmenu();
void fulloledupdate();
void drawMenuAction();
void drawSettingsAction();
void drawfunctype();
bool yesnoScreen(bool defaultval);
bool yesnoScreen(bool defaultval, char* line1);
bool yesnoScreen(bool defaultval, char* line1, char* line2);
String inputScreen(uint8_t maxLength, char* line);
String inputScreen(uint8_t maxLength, char* line, String regex);
String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty);
String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty, String regex);


#endif

