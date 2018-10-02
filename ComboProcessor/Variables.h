// Variables.h

#ifndef _VARIABLES_h
#define _VARIABLES_h

#include <BPLib.h>
#include <SdFs.h>
#include <Adafruit_SSD1306.h>
#include "USBMassStorage.h"
#include "Adafruit_MCP23008.h"
#include <ClickEncoder.h>
#include "I2CMatrix.h"
#include "Adafruit_SSD1306_EXT.h"
#include "USBHID.h"

enum OperateMode { CABLE, BLUETOOTH, CHARGING };

enum FunctionType {
	MEDIA, // Media hotkeys
	ANIME,  // Shortcuts for easy anime watching
	FNKEYS, // Function keys(Function key pressed)
	PROGRAMMING, // Programming keys
	PASSWORD // Password (With use of mifare card)
};

struct BluetoothAddress
{
	char name[30];
	char address[12];
};

struct Settings {
	BluetoothAddress bt_addressess[9]; // Holds 9 bluetooth addresses
	uint8_t bluetoothAutoConnect = 0; // byte from 0 to 9 of what address to choose. 0 is last connected
	float mouseaccel[2] = { 2,2 };
};
extern Settings settings;

extern Adafruit_MCP23008 expender;
extern Adafruit_MCP23008 oled_expender;
extern BPLib *bt;
extern ClickEncoder *encoder;
extern enum OperateMode operateMode;
extern SdFs SD;
extern Adafruit_SSD1306 main_oled;
extern Adafruit_SSD1306_EXT oled12;
extern Adafruit_SSD1306_EXT oled34;
extern Adafruit_SSD1306_EXT oled56;
extern Adafruit_SSD1306_EXT oled78;
extern Adafruit_SSD1306_EXT oled9;
extern uint8_t menuindex;
extern FunctionType funcType;
extern FunctionType oldFuncType;
extern USBHID HID;
extern HIDKeyboard Keyboard;
extern HIDMouse Mouse;
extern HIDConsumer Consumer;
extern USBMassStorage MassStorage;
extern I2CMatrixClass matrix;


extern float batteryvoltage;
extern bool bluetoothConnected;
extern bool sdInitialized; // Used in case the sd does not initialize and we do sd functions
extern int16_t currentEncPos;
extern int16_t lastEncPos;

void initVars();

#endif

