// KeyMacro.h

#ifndef _KEYMACRO_h
#define _KEYMACRO_h

#include <Arduino.h>

class KeyReport
{
 protected:


 public:
	 uint8_t modifier = 0;
	 uint8_t keys[6] = { 0,0,0,0,0,0 };
	void init();
	bool isConsumerKey();
	bool existsInList(uint8_t key);
};

#endif

