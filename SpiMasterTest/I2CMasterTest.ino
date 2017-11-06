/*
 Name:		SpiMasterTest.ino
 Created:	11/4/2017 11:33:50 AM
 Author:	Natsuki
*/

// the setup function runs once when you press reset or power the board
#include <Wire.h>
#include "SPI_anything.h"

const byte SLAVE_ADDRESS = 42;

HardWire HWire(1, I2C_FAST_MODE); // I2c1
typedef struct Key
{
	uint16_t code;
	uint8_t state;
	uint8_t num;
};

Key key;

void setup()
{
	Serial.begin(115200);
	Serial.println("Setting up i2c");
	HWire.begin();
	pinMode(PA0, INPUT_ANALOG);
	Serial.println("i2c setup done");
	randomSeed(analogRead(PA0));
	key.num = 1;
	Serial.println("Setup done");
}  // end of setup

void loop()
{
	key.code = random(0, 255);
	key.state = random(0, 3);
	Serial.print("Writing Key ");
	Serial.print(key.num);
	Serial.println(" to I2C slave");
	HWire.beginTransmission(SLAVE_ADDRESS);
	HWire.write(lowByte(key.code));
	HWire.write(highByte(key.code));
	HWire.write(key.state);
	HWire.write(key.num);
	HWire.endTransmission();
	delay(1000);  // for testing  

	if (key.num == 6) key.num = 1;
	else key.num++;
}  // end of loop
