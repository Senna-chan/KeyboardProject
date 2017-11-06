// I2CMatrix.h

#ifndef _I2CMATRIX_h
#define _I2CMATRIX_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include "Key.h"
#include "Adafruit_MCP23017.h"
#include <HID-Project.h>

#define OPEN LOW
#define CLOSED HIGH

typedef char KeypadEvent;
typedef unsigned int uint;
typedef unsigned long ulong;

// Made changes according to this post http://arduino.cc/forum/index.php?topic=58337.0
// by Nick Gammon. Thanks for the input Nick. It actually saved 78 bytes for me. :)
typedef struct {
	byte rows;
	byte columns;
} KeypadSize;

#define LIST_MAX 6		// Max number of keys on the active list.
#define MAPSIZE 8		// MAPSIZE is the number of rows (times 16 columns)
#define makeKeymap(x) ((KeyboardKeycode*)x)

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

//class Keypad : public Key, public HAL_obj {
class I2CMatrixClass : public Key {
public:
	I2CMatrixClass();
	I2CMatrixClass(KeyboardKeycode *userKeymap, Adafruit_MCP23017 rowChip, Adafruit_MCP23017 colChip, byte numRows, byte numCols);

	uint bitMap[MAPSIZE];	// 10 row x 16 column array of bits. Except Due which has 32 columns.
	Key key[LIST_MAX];
	unsigned long holdTimer;

	char getKey();
	bool getKeys();
	KeyState getState();
	void begin();
	bool isPressed(char keyChar);
	void setDebounceTime(uint);
	void setHoldTime(uint);
	void addEventListener(void(*listener)(char));
	int findInList(char keyChar);
	int findInList(int keyCode);
	char waitForKey();
	bool keyStateChanged();
	byte numKeys();
	virtual void pin_mode(byte pinNum, int mode, bool col) {
		if (mode == INPUT_PULLUP) {
			if(col){
				colChip.pinMode(pinNum, INPUT);
				colChip.pullUp(pinNum, HIGH);
			}
			else
			{
				rowChip.pinMode(pinNum, INPUT);
				rowChip.pullUp(pinNum, HIGH);
			}
			return;
		}
		if(col)
		{
			colChip.pinMode(pinNum, mode);
		}
		else
		{
			rowChip.pinMode(pinNum, mode);
		}
	}
	virtual void pin_write(byte pinNum, bool level, bool col) {
		if (col)
		{
			colChip.digitalWrite(pinNum, level);
		}
		else
		{
			rowChip.digitalWrite(pinNum, level);
		}
	}
	virtual int  pin_read(byte pinNum, bool col) {
		if (col)
		{
			colChip.digitalRead(pinNum);
		}
		else
		{
			rowChip.digitalRead(pinNum);
		}
	}
	virtual bool getBit(byte thebyte, int position)
	{
		return (1 == ((thebyte >> position) & 1));
	}

private:
	unsigned long startTime;
	KeyboardKeycode *keymap;
	Adafruit_MCP23017 rowChip;
	Adafruit_MCP23017 colChip;
	KeypadSize sizeKpd;
	uint debounceTime;
	uint holdTime;
	bool single_key;

	void scanKeys();
	bool updateList();
	void nextKeyState(byte n, boolean button);
	void transitionTo(byte n, KeyState nextState);
	void(*keypadEventListener)(char);
};

extern I2CMatrixClass I2CMatrix;

#endif

