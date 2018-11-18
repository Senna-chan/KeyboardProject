// 
// 
// 

#include "KeyReport.h"

void KeyReport::init()
{

}

bool KeyReport::isConsumerKey()
{
	return (modifier == 0xff);
}

bool KeyReport::existsInList(uint8_t key)
{
	for(auto& k : keys)
	{
		if (k == key) return true;
	}
	return false;
}

