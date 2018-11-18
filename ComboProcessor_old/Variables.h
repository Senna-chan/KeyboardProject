// Variables.h

#ifndef _VARIABLES_h
#define _VARIABLES_h

#include <BPLib.h>
#include <SdFs.h>
#include <Adafruit_SSD1306.h>
#include <USBMassStorage.h>
#include <Adafruit_MCP23008.h>
#include "I2CMatrix.h"
#include "Adafruit_SSD1306_EXT.h"
#include <USBHID.h>
#include <i2cEncoderLibV2.h>

enum OperateMode { CABLE, BLUETOOTH, CHARGING };

// enum FunctionType {
// 	STARTUP = -1,
// 	MEDIA, // Media hotkeys
// 	ANIME,  // Shortcuts for easy anime watching
// 	FNKEYS, // Function keys(Function key pressed)
// 	PROGRAMMING, // Programming keys
// };

struct ConnectionConfig
{
	char name[30];
	char address[12];
	float mouseaccel[2] = { 1.0,1.0 };
};

struct keyReport
{
	uint8_t modifier = 0;
	uint8_t keys[6] = {0,0,0,0,0,0};
};

struct Settings {
	String functypes[9] = { "MEDIA","PROGRAMMING","","","","","","","" };
	ConnectionConfig connectionconfigs[9]; // Holds 9 connection configs(name, bluetooth address, mouseacceleration)
	uint8_t bluetoothAutoConnect = 0; // byte from 0 to 9 of what address to choose. 0 is last connected
	float def_mouseaccel[2] = { 1.0, 1.0 };
	String lastconnected = "Desktop";
};
extern Settings settings;
extern ConnectionConfig currentConnectionConfig;

extern Adafruit_MCP23008 expender;
extern Adafruit_MCP23008 oled_expender;
extern BPLib *bt;
extern i2cEncoderLibV2 *encoder;
extern bool encoderInterupted;
extern enum OperateMode operateMode;
extern SdFs SD;
extern Adafruit_SSD1306 main_oled;
extern Adafruit_SSD1306_EXT oled12;
extern Adafruit_SSD1306_EXT oled34;
extern Adafruit_SSD1306_EXT oled56;
extern Adafruit_SSD1306_EXT oled78;
extern Adafruit_SSD1306_EXT oled9;
extern uint8_t menuindex;
extern uint8_t settingsindex;
extern String funcType;
extern String oldFuncType;
extern uint8_t funcindex;
extern USBHID HID;
extern HIDKeyboard Keyboard;
extern HIDMouse Mouse;
extern HIDConsumer Consumer;
extern USBMassStorage MassStorage;
extern I2CMatrixClass matrix;

extern keyReport keyMacros[9][9]; // This holds the macro keys. Max 9 key combinations per marco(Is this overkill?)


extern float batteryvoltage;
extern bool bluetoothConnected;
extern bool sdInitialized; // Used in case the sd does not initialize and we do sd functions

extern int16_t lastEncPos;
extern int16_t currentEncPos;

extern unsigned long seconds;

// Menu things
extern bool menuActive;
extern bool settingsActive;
extern unsigned long menuActivationTime;


extern keyReport curKeyReport; // Stubbern var

void initVars();



extern const char* MenuScreens[][2];
extern const char* SettingsScreens[][2];

#endif