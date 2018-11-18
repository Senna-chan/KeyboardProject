// 
// 
// 

#include "Variables.h"
#include "Adafruit_SSD1306_EXT.h"
#include <Adafruit_MCP23008.h>
#include "Config.h"


Adafruit_MCP23008 oled_expender = Adafruit_MCP23008();
Adafruit_SSD1306_EXT oled12 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 1);
Adafruit_SSD1306_EXT oled34 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 2);
Adafruit_SSD1306_EXT oled56 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 3);
Adafruit_SSD1306_EXT oled78 = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 4);
Adafruit_SSD1306_EXT oled9  = Adafruit_SSD1306_EXT(oled_expender, OLED_DC, OLED_RESET, 5);
keyReport keyMacros[9][9];
keyReport curKeyReport;
unsigned long seconds = 0;
bool menuActive = false;
bool settingsActive = false;
unsigned long menuActivationTime = seconds;
uint8_t funcindex = 0;
uint8_t settingsindex = 0;
ConnectionConfig currentConnectionConfig = settings.connectionconfigs[0];

const char* MenuScreens[][2]{
	{"Settings",""}
};

const char* SettingsScreens[][2]{
	{"Connection","Configs"},
	{"Mouse","Acceleration"}
};

void initVars()
{
	Serial.println("Setting up some vars");
	Settings settings;
	oled_expender.begin(B010);
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