/*********************************************************************
This is an example for Adafuit's Monochrome OLEDs based on SSD1306 drivers

Pick one up today in the adafruit shop!
------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if defined(__STM32F1__)
#define OLED_DC     PB8
#define OLED_CS     PA4
#define OLED_RESET  PB9
#elif defined(__AVR__)
#define OLED_DC     8
#define OLED_CS     10
#define OLED_RESET  9
#endif
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

// 'undefined'
const unsigned char myBitmap[] PROGMEM = {
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000,
	B000000, B00000111, B00000000,
	B000000, B00011111, B10000000,
	B000000, B00111111, B10000000,
	B000000, B01110111, B10000000,
	B011100, B01100000, B00000000,
	B111110, B11100000, B00001100,
	B111111, B11111111, B11111111,
	B111111, B11111111, B11111111,
	B111110, B00111000, B00001110,
	B011100, B00011000, B00001100,
	B000000, B00011101, B11100000,
	B000000, B00001111, B11100000,
	B000000, B00000111, B11100000,
	B000000, B00000001, B11100000,
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000,
	B000000, B00000000, B00000000
};

void setup() {
	display.begin(SSD1306_SWITCHCAPVCC); 

	display.clearDisplay(); // Make sure the display is cleared
							// Draw the bitmap:
							// drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
	display.drawBitmap(10, 10, myBitmap, 22, 22, WHITE);

	// Update the display
	display.display();
}


void loop() {

}