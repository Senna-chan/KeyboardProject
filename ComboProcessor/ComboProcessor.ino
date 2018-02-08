#include <eepromi2c_Anything.h>
#include "KeyboardCodes.h"
#include "ConsumerKeyCodes.h"
#include <USBHID.h>
#include <BPLib.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP23017.h>
#include <Wire.h>
#include "I2CMatrix.h"
#include "Key.h"
#include <TimedAction.h>
#include "PS2Mouse.h"
#include <ClickEncoder.h>
#include "CustomIcons.h" // Bitmaps for the Oled

#define analogReadVoltage(pin) analogRead(pin) * (3.3 / 4095)
#define between(value, min, max) ((value) < (max) && (value) > (min))
#define outside(value, min, max) ((value) > (max) || (value) < (min))
#define DATA_PIN PB13 // green
#define CLOCK_PIN PB14 // white
#define BATPIN PA1
#define MODESELECTPIN PA2

#define DEBUG
#define DEBUG_LEVEL "trace"
//#define ENCODER_ENABLED
#define MOUSE_ENABLED
#define KEYBOARD_ENABLED
#define HID_ENABLED
//#define OLED_ENABLED
#define EEPROM_ENABLED
#define BT_ENABLED

BPLib *bt;

#define ENC_SW PA7
#define ENC_DT PB0
#define ENC_CLK PB1
HardwareTimer timer(1);

#define OLED_DC     PB8
#define OLED_CS     PA4
#define OLED_RESET  PB9
Adafruit_SSD1306 oled(OLED_DC, OLED_RESET, OLED_CS);

ClickEncoder *encoder;
int16_t lastEncPos, currentEncPos;
const int numOfScreens = 10;
int currentScreen = 0;
String screens[numOfScreens][2] = { { "Function Mode" },{ "Active Mode" },{ "Active BTAddr" },
{ "Set BTAddr" },{ "Overload Temp.","degC" },{ "Accel Time", "Secs" },{ "Restart Time","Mins" },
{ "Analog Out. Curr.","mA" },{ "Input Temp.","degC" },{ "Run Time", "Hours" } };
int parameters[numOfScreens];
#define resetToHomeScreen 5000
bool checkResetHomeScreen, doValueChange;
enum FunctionType { MEDIA, ONEPRESS };
enum OperateMode { CABLE, BLUETOOTH, CHARGING };
enum KeyboardLeds : uint8_t {
	LED_NUM_LOCK = (1 << 0),
	LED_CAPS_LOCK = (1 << 1),
	LED_SCROLL_LOCK = (1 << 2)
};
Adafruit_MCP23017 rowChip, colChip;
const byte ROWS = 8;
const byte COLS = 16;
uint8_t keys[ROWS][COLS] =
{
	{ HID_KEYBOARD_LEFT_ALT,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_B,KEY_N,HID_KEYBOARD_DOWNARROW,HID_KEYBOARD_LEFTARROW,HID_KEYBOARD_RIGHTARROW,HID_KEYPAD_SUBTRACT,KEY_SPACE,KEY_SLASH,KEY_RESERVED,KEY_RESERVED },
	{ KEY_RESERVED,HID_KEYBOARD_RIGHT_SHIFT,HID_KEYBOARD_RIGHT_CONTROL,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_M,KEY_NUM_LOCK,HID_KEYBOARD_PAUSE,HID_KEYPAD_DIVIDE,HID_KEYPAD_MULTIPLY,KEY_ENTER,KEY_RESERVED,KEY_PERIOD,KEY_COMMA },
	{ KEY_RESERVED,(KeyboardKeycode)0xF7,(KeyboardKeycode)0xF3,KEY_A,KEY_S,KEY_D,KEY_F,KEY_J,HID_KEYPAD_1_AND_END,HID_KEYPAD_ENTER,HID_KEYPAD_2_AND_DOWN_ARROW,HID_KEYPAD_3_AND_PAGE_DOWN,KEY_BACKSLASH,KEY_SEMICOLON,KEY_L,KEY_K },
	{ KEY_RESERVED,HID_KEYBOARD_LEFT_SHIFT,(KeyboardKeycode)0xF2,HID_KEYBOARD_TAB,HID_KEYBOARD_CAPS_LOCK,HID_KEYBOARD_F3,KEY_T,KEY_Y,HID_KEYPAD_4_AND_LEFT_ARROW,KEY_RESERVED,HID_KEYPAD_5,HID_KEYPAD_6_AND_RIGHT_ARROW, HID_KEYBOARD_DELETE /*BACKSPACE*/  ,KEY_LEFT_BRACE,HID_KEYBOARD_F7,KEY_RIGHT_BRACE },
	{ KEY_SCROLL_LOCK,(KeyboardKeycode)0xF6,(KeyboardKeycode)0xF1,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_U,HID_KEYPAD_7_AND_HOME,HID_KEYPAD_ADD,HID_KEYPAD_8_AND_UP_ARROW,HID_KEYPAD_9_AND_PAGE_UP,KEY_RESERVED,KEY_P,KEY_O,KEY_I },
	{ HID_KEYBOARD_LEFT_ALT,(KeyboardKeycode)0xF8,(KeyboardKeycode)0xF4,HID_KEYBOARD_ESCAPE,KEY_RESERVED,HID_KEYBOARD_F4,KEY_G,KEY_H,KEY_RESERVED,HID_KEYBOARD_UPARROW,HID_KEYPAD_0_AND_INSERT,HID_KEYPAD_PERIOD_AND_DELETE,HID_KEYBOARD_F5,KEY_QUOTE,KEY_RESERVED,HID_KEYBOARD_F6 },
	{ KEY_PRINTSCREEN,(KeyboardKeycode)0xF5,(KeyboardKeycode)0xF0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_7,HID_KEYBOARD_F11,KEY_RESERVED,HID_KEYBOARD_F12,KEY_RESERVED,HID_KEYBOARD_F10,KEY_0,KEY_9,KEY_8 },
	{ (KeyboardKeycode)0xF9,KEY_RESERVED,HID_KEYBOARD_LEFT_CONTROL,KEY_TILDE,HID_KEYBOARD_F1,HID_KEYBOARD_F2,KEY_5,KEY_6,  76  /*DELETE*/,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,HID_KEYBOARD_F9,KEY_MINUS,HID_KEYBOARD_F8,KEY_EQUAL }
};
uint16_t mediakeys[9]{ HID_CONSUMER_VOLUME_DECREMENT, HID_CONSUMER_MUTE, HID_CONSUMER_VOLUME_INCREMENT, MEDIA_PREVIOUS, MEDIA_PLAY_PAUSE, MEDIA_NEXT, /*Opens media player*/ HID_CONSUMER_AL_AUDIO_PLAYER, };

PS2Mouse ps2_mouse(CLOCK_PIN, DATA_PIN);
bool leftMouseButton = 0;
bool rightMouseButton = 0;
bool capsState = false ,numState = false, scrollState = false;
TimedAction doMouseStuffAction = TimedAction(); 
TimedAction checkBatteryAction = TimedAction();
I2CMatrixClass matrix;
float batteryVoltage;
FunctionType funcType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
FunctionType oldFuncType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys

#define FORCEOPMODE
OperateMode operateMode;






#ifdef HID_ENABLED
const uint8_t reportDescription[] = {
	HID_MOUSE_REPORT_DESCRIPTOR(HID_MOUSE_REPORT_ID),
	HID_KEYBOARD_REPORT_DESCRIPTOR(HID_KEYBOARD_REPORT_ID),
	HID_CONSUMER_REPORT_DESCRIPTOR(HID_CONSUMER_REPORT_ID)
};
uint8 oldLeds = 0;

HIDConsumer Consumer;
#endif
uint8 leds;

struct EEPROMSettings {
	byte settingsVersion;
	char btaddr1[12];
	char btaddr2[12];
	char btaddr3[12];
	char btaddr4[12];
	byte mouseaccel[2];// X and Y seperate
	byte reconnectaddr; // number of bt addr
} EEPROMSettings;
#define EEPROMSettingsVersion 2
struct Settings {
	float mouseaccelx;
	float mouseaccely;
} settings;

bool GetBit(byte thebyte, int position)
{
	return (1 == ((thebyte >> position) & 1));
}
void printBits(int var) {
	unsigned int test;
	if (var < 255) {
		test = 0x80;
	}
	else {
		test = 0x8000;
	}
	for (test; test; test >>= 1) {
		Serial.write(var  & test ? '1' : '0');
	}
	Serial.println();
}

void checkBattery() {
	batteryVoltage = analogReadVoltage(BATPIN) * 2; // Cutting voltage in half with voltage divider to get a range of 1.5 to 2.1 volt. > 3,3 volt on ADC will kill the STM32
	updateOled();
}

void PressMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if(!bt->mouseIsPressed(button)) bt->mousePress(button);
	}
	else if (operateMode == CABLE) {
		if (!Mouse.isPressed(button)) Mouse.press(button);
	}
}

void ReleaseMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if(!bt->mouseIsPressed(button))  bt->mouseRelease(button);
	}
	else if (operateMode == CABLE) {
		if (Mouse.isPressed(button)) Mouse.release(button);
	}
}

void MoveMouse(byte x, byte y) {
	x = x * settings.mouseaccelx;
	y = y * settings.mouseaccely;
	if (operateMode == BLUETOOTH) {
		bt->mouseMove(x, y);
	}
	else if (operateMode == CABLE) {
		Mouse.move(x, y);
	}
}

void doMouseStuff()
{
#ifdef MOUSE_ENABLED
	MouseData data = ps2_mouse.readData();
	leftMouseButton = GetBit(data.status, 0);
	rightMouseButton = GetBit(data.status, 1);
	#ifdef HID_ENABLED
		if (leftMouseButton) {
			PressMouse(MOUSE_LEFT);
		}
		else {
			ReleaseMouse(MOUSE_LEFT);
		}

		if (rightMouseButton) {
			PressMouse(MOUSE_RIGHT);
		}
		else {
			ReleaseMouse(MOUSE_RIGHT);
		}
		if (data.position.x != 0 || data.position.y != 0) {
			MoveMouse(data.position.x, data.position.y *-1);
		}
	#endif
#endif
}

void updateOled() {
#ifndef OLED_ENABLED
	return;
#endif
	oled.clearDisplay();
	if (operateMode == BLUETOOTH) {
		oled.drawBitmap(0, 0, bluetooth, 16, 16, WHITE);
	}
	else if (operateMode == CABLE) {
		oled.drawBitmap(0, 0, usb, 16, 16, WHITE);
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
void checkForLeds() {
	if (operateMode == CABLE) {
		leds = Keyboard.getLEDs();
	}
	else {
		//leds = bt->keyboardGetLeds();
	}
	if (leds != oldLeds) {
		printBits(leds);
		if (leds & LED_CAPS_LOCK && !capsState) {
			rowChip.digitalWrite(15, LOW);
			capsState = true;
		}
		else if (capsState) {
			rowChip.digitalWrite(15, HIGH);
			capsState = false;
		}
		if (leds & LED_NUM_LOCK && !numState) {
			rowChip.digitalWrite(14, LOW);
			numState = true;
		}
		else if (numState) {
			rowChip.digitalWrite(14, HIGH);
			numState = false;
		}
		if (leds & LED_SCROLL_LOCK && !scrollState) {
			rowChip.digitalWrite(13, LOW);
			scrollState = true;
		}
		else if (scrollState) {
			rowChip.digitalWrite(13, HIGH);
			scrollState = false;
		}
		oldLeds = leds;
	}
}


void timerIsr() {
	encoder->service();
}

void setup() {
	Serial.begin(115200);
	Serial.println(F("Starting custom keyboard"));
#if defined(KEYBOARD_ENABLED) || defined(EEPROM_ENABLED)
	Serial.println(F("Initializing Wire"));
	Wire.setClock(100000);
	delay(50);
	Wire.begin();
#endif
#ifdef EEPROM_ENABLED
	Serial.println(F("Reading settings"));
	eeRead(0, EEPROMSettings);
	if (EEPROMSettings.settingsVersion != EEPROMSettingsVersion) {
		Serial.println(F("EEPROM Settings are not valid"));
		EEPROMSettings.settingsVersion = EEPROMSettingsVersion;
		EEPROMSettings.mouseaccel[0] = 100;
		EEPROMSettings.mouseaccel[1] = 100;
		EEPROMSettings.reconnectaddr = 0;
		eeWrite(0, EEPROMSettings);
	}
	settings.mouseaccelx = EEPROMSettings.mouseaccel[0] / 100;
	settings.mouseaccely = EEPROMSettings.mouseaccel[1] / 100;
#endif
#ifdef KEYBOARD_ENABLED
	Serial.println(F("Initializing keyboard wire things"));
	delay(50);
	rowChip.begin(B000);
	delay(50);
	colChip.begin(B100);
	

	rowChip.pinMode(13, OUTPUT);
	rowChip.pinMode(14, OUTPUT);
	rowChip.pinMode(15, OUTPUT);
	rowChip.digitalWrite(13, HIGH);
	rowChip.digitalWrite(14, HIGH);
	rowChip.digitalWrite(15, HIGH);
	Serial.println(F("Wire initialized"));
#endif
#ifdef BT_ENABLED
	Serial.println(F("Initializing BT Chip"));
	Serial2.begin(115200);
	bt = new BPLib(Serial2);
	if (EEPROMSettings.reconnectaddr == 0) bt->reconnect();
	if (EEPROMSettings.reconnectaddr == 1) bt->reconnect(EEPROMSettings.btaddr1);
	if (EEPROMSettings.reconnectaddr == 2) bt->reconnect(EEPROMSettings.btaddr2);
	if (EEPROMSettings.reconnectaddr == 3) bt->reconnect(EEPROMSettings.btaddr3);
	if (EEPROMSettings.reconnectaddr == 4) bt->reconnect(EEPROMSettings.btaddr4);
#endif
#ifdef HID_ENABLED
	Serial.println(F("Initializing HID"));

	//Reset the USB interface on generic boards - developed by Victor PV
	gpio_set_mode(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit, GPIO_OUTPUT_PP);
	gpio_write_bit(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit, 0);

	for (volatile unsigned int i = 0; i<512; i++);// Only small delay seems to be needed, and USB pins will get configured in Serial.begin
	gpio_set_mode(PIN_MAP[PA12].gpio_device, PIN_MAP[PA12].gpio_bit, GPIO_INPUT_FLOATING);

	USBHID.begin(reportDescription, sizeof(reportDescription));
	Keyboard.begin(); // Enables leds
	Keyboard.setKeyMapMode(true);
	Serial.println(F("HID Initialized"));
#endif
#ifdef MOUSE_ENABLED
	Serial.println(F("Initializing PS2 Mouse"));
	ps2_mouse.initialize(); // If it is not connected DO NOT CALL ANY PS2 MOUSE FUNCTION
	doMouseStuffAction = TimedAction(10, *doMouseStuff);
	Serial.println(F("PS2 Mouse Initialized"));
#endif // MOUSE_ENABLED

#ifdef ENCODER_ENABLED
	Serial.println(F("Initializing Encoder"));
	encoder = new ClickEncoder(ENC_DT, ENC_CLK, ENC_SW, 4);
	encoder->setAccelerationEnabled(false);
	encoder->setDoubleClickEnabled(true);
	timer.pause();
	timer.setPeriod(1000);
	// Set up an interrupt on channel 1
	timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
	timer.attachCompare1Interrupt(timerIsr);
	timer.refresh(); // Refresh the timer's count, prescale, and overflow
	timer.resume(); // Start the timer counting
	lastEncPos = -1;
	Serial.println(F("Encoder Initialized"));
#endif // ENCODER_ENABLED

#ifdef OLED_ENABLED
	Serial.println(F("Initializing Oled"));
	oled.begin();
	oled.display();
	delay(1000);
	oled.clearDisplay();
	oled.println(F("Oled Initialized"));
	oled.display();
	delay(1000);
	oled.clearDisplay();
	oled.display();
	Serial.println(F("Oled Initialized"));
#endif

#ifdef KEYBOARD_ENABLED
	Serial.println(F("Initializing Matrix"));
	matrix = I2CMatrixClass(makeKeymap(keys), rowChip, colChip, ROWS, COLS);
	matrix.begin();
	matrix.setDebounceTime(2);
	Serial.println(F("Matrix Initialized"));
#endif
#ifdef FORCEOPMODE
	operateMode = BLUETOOTH;
#endif
	Serial.println(F("Initializing Done, Keyboard is ready"));
}

void handleEncoder() {
	currentEncPos += encoder->getValue();

	if (currentEncPos != lastEncPos) {
		/*if (currentEncPos < lastEncPos) {
			if (doValueChange) {
				parameters[currentScreen]--;
			}
			else {
				if (currentScreen == 0) {
					currentScreen = numOfScreens - 1;
				}
				else {
					currentScreen--;
				}
			}
		}
		else if (currentEncPos > lastEncPos) {
			if (doValueChange) {
				parameters[currentScreen]++;
			}
			else {
				if (currentScreen == numOfScreens - 1) {
					currentScreen = 0;
				}
				else {
					currentScreen++;
				}
			}
		}
		printScreen();*/
		lastEncPos = currentEncPos;
		Serial.print(F("Encoder Value: "));
		Serial.println(currentEncPos);
	}

	ClickEncoder::Button b = encoder->getButton();
	if (b != ClickEncoder::Open) {
		Serial.print(F("Button: "));
#define VERBOSECASE(label) case label: Serial.println(#label); break;
		switch (b) {
			VERBOSECASE(ClickEncoder::Pressed)
			VERBOSECASE(ClickEncoder::Held)
			VERBOSECASE(ClickEncoder::Released)
			VERBOSECASE(ClickEncoder::Clicked)
			VERBOSECASE(ClickEncoder::DoubleClicked)
		}
		if (b == ClickEncoder::Clicked) {
			/*if (!doValueChange) {
			doValueChange = true;
			lcd.blink();
			}
			else {
			doValueChange = false;
			lcd.noBlink();
			}*/
		}
	}
}


void pressKeyboardKey(uint8_t key) {
	if (operateMode == BLUETOOTH) {
		bt->keyboardPress(key);
#if defined(DEBUG)
		if (DEBUG_LEVEL == "trace") {
			for (int i = 0; i < 6; i++) {
				Serial.print(F("\tKey "));
				Serial.print(i);
				Serial.print(F(":\t"));
				Serial.print(bt->keyReport.keys[i]);
			}
			Serial.println();
		}
#endif
	}
	else if (operateMode == CABLE) {
		Keyboard.press(key);
	}
}
void releaseKeyboardKey(uint8_t key) {
	if (operateMode == BLUETOOTH) {
		bt->keyboardRelease(key);
#if defined(DEBUG)
		if (DEBUG_LEVEL == "trace") {
			for (int i = 0; i < 6; i++) {
				Serial.print(F("\tKey "));
				Serial.print(i);
				Serial.print(F(":\t"));
				Serial.print(bt->keyReport.keys[i]);
			}
			Serial.println();
		}
#endif
	}
	else if (operateMode == CABLE) {
		Keyboard.release(key);
	}
}
void writeKeyboardKey(uint8_t key) {
	if (operateMode == BLUETOOTH) {
		bt->keyboardWrite(key);
	}
	else if (operateMode == CABLE) {
		Keyboard.write(key);
	}
}
void pressConsumerKey(uint16_t key) {
	if (operateMode == BLUETOOTH) {
		bt->sendConsumer(key);
	}
	else if (operateMode == CABLE) {
		Consumer.press(key);
	}
}
void releaseConsumerKey(uint16_t key) {
	if (operateMode == BLUETOOTH) {
		bt->releaseConsumer();
	}
	else if (operateMode == CABLE) {
		Consumer.release();
	}
}
void writeConsumerKey(uint16_t key) {
	if (operateMode == BLUETOOTH) {
		bt->writeConsumer(key);
	}
	else if (operateMode == CABLE) {
		Consumer.write(key);
	}
}

void processPressedKey(byte key) {
	if (key == 0xF0) {
		oldFuncType = funcType;
		funcType = ONEPRESS;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		switch (funcType) {
		case MEDIA:
			pressConsumerKey(mediakeys[func]);
			break;
		case ONEPRESS:
			break;
		}
		return;
	}
	if (funcType == ONEPRESS) {
		switch (key)
		{
		case KEY_UP:
			writeConsumerKey(CONSUMER_BRIGHTNESS_UP);
			break;
		case KEY_DOWN:
			writeConsumerKey(CONSUMER_BRIGHTNESS_DOWN);
			break;
		default:
			break;
		}
		return;
	}
	// From here on we know it is not a special key so we can use some special things without directly pressing the button
	pressKeyboardKey(key);
}
void processReleasedKey(uint8_t key) {
	if (key == 0xF0) {
		funcType = oldFuncType;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		switch (funcType) {
		case MEDIA:
			Consumer.release();
			break;
		case ONEPRESS:
			if (operateMode == CABLE) {
			}
			break;
		}
		return;
	}
	releaseKeyboardKey(key);
}

void checkKeys() {
	if (matrix.getKeys())
	{
		for (int i = 0; i<LIST_MAX; i++)   // Scan the whole key list.
		{
			if (matrix.key[i].stateChanged)   // Only find keys that have changed state.
			{
				switch (matrix.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
				case PRESSED:
					#ifdef DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\tpressed"));
					#endif
					#ifdef HID_ENABLED
						processPressedKey(matrix.key[i].kcode);
					#endif
					break;
				case RELEASED:
					#ifdef DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\treleased"));
					#endif
					#ifdef HID_ENABLED
						processReleasedKey(matrix.key[i].kcode);
					#endif
					break;
				case HOLD:
					#ifdef DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\tholded"));
					#endif
				case IDLE:
					break;
				}
			}
		}
	}
}
void loop() {
	if (Serial.available() > 0) {
		String line = Serial.readStringUntil('\r\n');
		if (line.startsWith("echo")) {
			CompositeSerial.println(line);
		}
	}
#ifndef FORCEOPMODE
	if (analogReadVoltage(MODESELECTPIN) > 3) {
		#ifdef BT_ENABLED
			operateMode = BLUETOOTH;
		#else
			operateMode = CABLE;
		#endif
	}
	else if (analogReadVoltage(MODESELECTPIN) < 1) {
		operateMode = CHARGING;
	}
	else {
		operateMode = CABLE;
	}
#endif

	checkBatteryAction.check();
	#ifdef MOUSE_ENABLED
		doMouseStuffAction.check();
	#endif
	
	#ifdef KEYBOARD_ENABLED
		#ifdef HID_ENABLED
			checkForLeds();
		#endif
		checkKeys();
	#endif

	#ifdef ENCODER_ENABLED
		handleEncoder();
	#endif
}