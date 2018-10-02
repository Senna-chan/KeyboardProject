/*
 Name:		InterruptMouseTest.ino
 Created:	9/2/2018 8:27:09 PM
 Author:	Natsuki
*/

// the setup function runs once when you press reset or power the board


#include <PS2Mouse.h>
#define DATA_PIN			PB12 // green
#define CLOCK_PIN			PB13 // white


PS2Mouse ps2_mouse(CLOCK_PIN, DATA_PIN);

void printBits(long var, bool newLine) {
	unsigned int test;
	if (var <= UINT8_MAX) {
		test = 0x80;
	}
	else if(var < UINT16_MAX){
		test = 0x8000;
	}
	else if(var < UINT32_MAX)
	{
		test = 0x800000;
	}
	for (test; test; test >>= 1) {
		Serial.write(var  & test ? '1' : '0');
	}
	if (newLine)
		Serial.println();
}

void setup() {
	Serial.begin(115200);
	Serial.println(F("Interupt based mouse test"));
	ps2_mouse.initialize();
	ps2_mouse.setSampleRate(200);
}

// the loop function runs over and over again until power down or reset
void loop() {
	MouseData data = ps2_mouse.readData();
	printBits(data.status, false);
	Serial.print("\tx=");
	Serial.print(data.position.x);
	Serial.print("\ty=");
	Serial.print(data.position.y);
	Serial.print("\twheel=");
	Serial.print(data.wheel);
	Serial.println();
}
