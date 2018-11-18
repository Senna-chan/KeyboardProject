#ifndef _CONFIG_H
#define _CONFIG_H

#include <SdFs.h>

#define DEBUG				1
#define DEBUGKEYBOARD		0
#define DEBUGMOUSE			0
#define ENCODER_ENABLED		1
#define MOUSE_ENABLED		1
#define KEYBOARD_ENABLED	1
#define HID_ENABLED			1
#define OLED_ENABLED		1
#define BT_ENABLED			1
#define MASS_ENABLED		1 // Mass storage
#define DUCKY_ENABLED		0 // Duckyscripts enabled



// Mouse
#define DATA_PIN			PB12 // green
#define CLOCK_PIN			PB13 // white

// Bluetooth


#define BATPIN				PA1

// MCP23008
#define MCPINT				PB14
#define BTMODEPIN			0
#define USBMODEPIN			1
#define SDUSBPIN			2

#define BT_CONNECTED_LED	7
#define INIT_LED			8

// SD
#define SD_CS				PA4
#define SD_CD				PB4 // Card detect	
#define SD_CONFIG_STARTUP	SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(5))

// ENCODER
#define ENC_INT				PB5 // Encoder interrupt pin
#define ENC_ADDR			0x41
// Oled
#define OLED_DC				PB8
#define OLED_CS				PB0
#define OLED_RESET			PB9


// Seconds things
const uint8_t menuReturnSeconds = 5;

#endif // _CONFIG_H