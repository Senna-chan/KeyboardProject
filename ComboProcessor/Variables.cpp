// 
// 
// 

#include "Variables.h"
#include "Adafruit_SSD1306.h"
#include <Adafruit_MCP23008.h>
#include "Config.h"
#include <SPI.h>


Adafruit_SSD1306 oled12		(128, 32, &SPI, OLED_DC, OLED_RESET, 1);
Adafruit_SSD1306 oled34		(128, 32, &SPI, OLED_DC, OLED_RESET, 2);
Adafruit_SSD1306 oled56		(128, 32, &SPI, OLED_DC, OLED_RESET, 3);
Adafruit_SSD1306 oled78		(128, 32, &SPI, OLED_DC, OLED_RESET, 4);
Adafruit_SSD1306 oled9		(128, 32, &SPI, OLED_DC, OLED_RESET, 5);
Adafruit_SSD1306 main_oled	(128, 64, &SPI, OLED_DC, OLED_RESET, OLED_CS);
KeyReport keyMacros[9][macroKeyDepth];
KeyReport curKeyReport;
unsigned long seconds = 0;
bool menuActive = false;
bool settingsActive = false;
unsigned long menuActivationTime = seconds;
uint8_t funcindex = 0;
uint8_t settingsindex = 0;
ConnectionConfig currentConnectionConfig = settings.connectionconfigs[0];

const char* MenuScreens[][2]{
	{"Return"},
	{"Settings",""},
	{"Connect to"}
};

const char* SettingsScreens[][2]{
	{"Return",""},
	{"Connection","Configs"},
	{"Mouse","Acceleration"}
};

void initVars()
{
	Serial.println("Setting up some vars");
	Settings settings;
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
	delay(1000);
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
	delay(1000);
	Serial.println("Vars setted up");
}