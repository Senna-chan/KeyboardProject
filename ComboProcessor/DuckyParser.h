// DuckyParser.h

#ifndef _DUCKYPARSER
#define _DUCKYPARSER

#include <Arduino.h>

class DuckyParser
{
  private:
	void ParseLine(String l);
	void Press(String b);
	int delayTime = 0;
	int sdEnable = 0;
 public:
	bool init(int sdPin);
	void ExecDucky(int file);
	void ExecDucky(String file);
};


#endif

