// Helpers.h

#ifndef _HELPERS_h
#define _HELPERS_h
#include <Arduino.h>
#include <i2cEncoderLibV2.h>

#define analogReadVoltage(pin) analogRead(pin) * (3.3 / 4095)
#define between(value, min, max) ((value) < (max) && (value) > (min))
#define outside(value, min, max) ((value) > (max) || (value) < (min))
#define VERBOSECASELN(label) case label: Serial.println(#label); break;
#define VERBOSECASE(label) case label: Serial.print(#label); break;

char* appendCharToCharArray(char* array, char a);
uint8_t* read1bitBMP(char *fname, int* _w, int* _h);
void handleEncoder(); // Function is in helpers because multiple files need the encoder
String getStringPartByNr(String data, char separator, int index);

#endif

