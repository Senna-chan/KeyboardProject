// 
// 
// 

#include "Variables.h"
#include "Adafruit_SSD1306_EXT.h"
#include <Adafruit_MCP23008.h>
#include "Config.h"


void initVars()
{
	Adafruit_MCP23008 oled_expender = Adafruit_MCP23008();
	Adafruit_SSD1306_EXT oled12 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 1);
	Adafruit_SSD1306_EXT oled34 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 2);
	Adafruit_SSD1306_EXT oled56 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 3);
	Adafruit_SSD1306_EXT oled78 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 4);
	Adafruit_SSD1306_EXT oled9 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 5);
	Serial.println("Setting up some vars");
	Settings settings;
	oled_expender.begin(B010);
	gpio_set_mode(GPIOB, 1, GPIO_OUTPUT_PP);
	gpio_write_bit(GPIOB, 1, LOW);
	delay(1);
	gpio_write_bit(GPIOB, 1, HIGH);

	oled12.begin();
	oled12.display();
	oled34.begin();
	oled34.display();
	oled56.begin();
	oled56.display();
	oled78.begin();
	oled78.display();
	oled9.begin();
	oled9.display();
	delay(2000);
	oled12.clearDisplay();
	oled12.display();
	oled34.clearDisplay();
	oled34.display();
	oled56.clearDisplay();
	oled56.display();
	oled78.clearDisplay();
	oled78.display();
	oled9 .clearDisplay();
	oled9.display();
	gpio_write_bit(GPIOB, 1, LOW);
	Serial.println("Vars setted up");
}