#include <Wire.h>
#include "SPI_anything.h"

// create a structure to store the different data values:
typedef struct Key
{
	uint16_t code;
	uint8_t state;
	uint8_t num;
};

const byte MY_ADDRESS = 42;
volatile Key key;
volatile bool haveData = false;
void setup()
{
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
	Serial.println("Seting up");
	Wire.begin(MY_ADDRESS);
	
	Wire.onReceive(receiveEvent);
	Serial.println("Setup done");
}  // end of setup

void loop()
{
	if (haveData)
	{
		Serial.print("KeyCode: ");
		Serial.print(key.code);
		Serial.print("\tKeyState: ");
		Serial.print(key.state);
		Serial.print("\tKeyNum: ");
		Serial.print(key.num);
		Serial.println();
		haveData = false;
	}
}  // end of loop

void receiveEvent(int howMany)
{
	if (howMany >= 4)
	{
		I2C_readAnything(key);
		haveData = true;
	}  // end if have enough data
}  // end of receiveEvent