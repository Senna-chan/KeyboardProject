/*
 Name:		OledTest.ino
 Created:	12/2/1617 8:16:57 PM
 Author:	Natsuki
*/
#include <SPI.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "CustomIcons.h" // Bitmaps for the Oled

#define analogReadVoltage(pin) analogRead(pin) * (3.3 / 4095)
#define between(value, min, max) ((value) < (max) && (value) > (min))

#define BATPIN PA1
#define MODESELECTPIN PA2

#if defined(__STM32F1__)
#define OLED_DC     PB8
#define OLED_CS     PA4
#define OLED_RESET  PB9
#elif defined(__AVR__)
#define OLED_DC     8
#define OLED_CS     10
#define OLED_RESET  9
#endif
Adafruit_SSD1306 oled(OLED_DC, OLED_RESET, OLED_CS);

enum OperateMode { CABLE, BLUETOOTH, CHARGING };
OperateMode operateMode;
float batteryVoltage;

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(BATPIN, INPUT_ANALOG);
	pinMode(MODESELECTPIN, INPUT_ANALOG);
	Serial1.begin(115200);
	Serial1.println(F("Initializing Oled"));
	oled.begin();
	oled.display();
	delay(1600);

	oled.setTextSize(1);
	oled.setTextColor(WHITE);
	// Clear the buffer.
	oled.clearDisplay();

	// draw a single pixel
	oled.drawPixel(10, 10, WHITE);
	// Show the display buffer on the hardware.
	// NOTE: You _must_ call display after making any drawing commands
	// to make them visible on the display hardware!
	oled.display();
	delay(200);
	oled.clearDisplay();
	oled.println(F("Oled Initialized"));
	oled.display();
	delay(1000);
	oled.clearDisplay();
	oled.display();
	Serial1.println(F("Oled Initialized"));
}

void updateOled() {
	oled.clearDisplay();
	if (operateMode == BLUETOOTH) {
		oled.drawBitmap(0, 0, bluetooth, 16, 16, WHITE);
	}
	else if (operateMode == CABLE) {
		oled.drawBitmap(0,0, usb, 16, 16, WHITE);
	}
	else if (operateMode == CHARGING) {
		oled.drawBitmap(0, 0, charging, 16, 16, WHITE);
	}
	if (batteryVoltage > 4) {
		oled.drawBitmap(108, 0, battery_full, 16, 16, WHITE);
	}
	else if (between(batteryVoltage, 3.75, 4)) {
		oled.drawBitmap(108, 0, battery_three_quarters, 16, 16, WHITE);
	}
	else if (between(batteryVoltage, 3.3, 3.75)) {
		oled.drawBitmap(108, 0, battery_half, 16, 16, WHITE);
	}
	else if (between(batteryVoltage, 3, 3.3)) {
		oled.drawBitmap(108, 0, battery_quarter, 16, 16, WHITE);
	}
	else if (batteryVoltage < 3) {
		oled.drawBitmap(108, 0, battery_empty, 16, 16, WHITE);
	}
	oled.setCursor(88, 4);
	oled.println(batteryVoltage, WHITE);
	oled.display();
}

// the loop function runs over and over again until power down or reset
void loop() {
	batteryVoltage = analogReadVoltage(BATPIN) * 2; // Cutting voltage in half with voltage divider to get a range of 1.5 to 2.1 volt. > 3,3 volt on ADC will kill the STM32
	if (analogReadVoltage(MODESELECTPIN) > 3) {
		operateMode = BLUETOOTH;
	}
	else if (analogReadVoltage(MODESELECTPIN) < 1) {
		operateMode = CHARGING;
	}
	else {
		operateMode = CABLE;
	}
	updateOled();
	delay(500);
}
