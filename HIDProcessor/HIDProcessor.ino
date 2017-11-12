#include <AltSoftSerial.h>
#include <BPLib.h>
#include <HID-Settings.h>
#include <HID-Project.h>
#include <DuckyParser.h>
#include <Wire.h>
#include <TimedAction.h>
#include "SerialAnything.h"

#define DATA_PIN 5 // green
#define CLOCK_PIN 6 // white
#define SD_ENABLE 10
#define BATPIN A0
#define analogReadVoltage(pin) analogRead(pin) * (5.0 / 1023)

//AltSoftSerial btSerial = AltSoftSerial(4, 5);
//BPLib *bt = new BPLib(btSerial);
//HardwareSerial &Serial1 = Serial1;
enum FunctionType {NORMAL, MEDIA, ONEPRESS };
enum OperateMode{NONE, USB, BLUETOOTH, CHARGING};
typedef enum { IDLE, PRESSED, HOLD, RELEASED } KeyState;
typedef struct {
	int8_t x, y;
} Position;

typedef struct {
	byte status;
	Position position;
	uint8_t wheel;
} MouseData;
DuckyParser duckyParser;
const byte ROWS = 8;
const byte COLS = 16;
KeyboardKeycode keys[ROWS][COLS] =
{
	{ KEY_RIGHT_ALT,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_B,KEY_N,KEY_DOWN_ARROW,KEY_LEFT_ARROW,KEY_RIGHT_ARROW,HID_KEYPAD_SUBTRACT,KEY_SPACE,KEY_SLASH,KEY_RESERVED,KEY_RESERVED },
	{ KEY_RESERVED,KEY_RIGHT_SHIFT,KEY_RIGHT_CTRL,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_M,KEY_NUM_LOCK,KEY_PAUSE,HID_KEYPAD_DIVIDE,HID_KEYPAD_MULTIPLY,KEY_ENTER,KEY_RESERVED,KEY_PERIOD,KEY_COMMA },
	{ KEY_RESERVED,(KeyboardKeycode)0xF7,(KeyboardKeycode)0xF3,KEY_A,KEY_S,KEY_D,KEY_F,KEY_J,HID_KEYPAD_1_AND_END,HID_KEYPAD_ENTER,HID_KEYPAD_2_AND_DOWN_ARROW,HID_KEYPAD_3_AND_PAGE_DOWN,KEY_BACKSLASH,KEY_SEMICOLON,KEY_L,KEY_K },
	{ KEY_RESERVED,KEY_LEFT_SHIFT,(KeyboardKeycode)0xF2,KEY_TAB,KEY_CAPS_LOCK,KEY_F3,KEY_T,KEY_Y,HID_KEYPAD_4_AND_LEFT_ARROW,KEY_RESERVED,HID_KEYPAD_5,HID_KEYPAD_6_AND_RIGHT_ARROW,KEY_BACKSPACE,KEY_LEFT_BRACE,KEY_F7,KEY_RIGHT_BRACE },
	{ KEY_SCROLL_LOCK,(KeyboardKeycode)0xF6,(KeyboardKeycode)0xF1,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_U,HID_KEYPAD_7_AND_HOME,HID_KEYPAD_ADD,HID_KEYPAD_8_AND_UP_ARROW,HID_KEYPAD_9_AND_PAGE_UP,KEY_RESERVED,KEY_P,KEY_O,KEY_I },
	{ KEY_LEFT_ALT,(KeyboardKeycode)0xF8,(KeyboardKeycode)0xF4,KEY_ESC,KEY_RESERVED,KEY_F4,KEY_G,KEY_H,KEY_RESERVED,KEY_UP_ARROW,HID_KEYPAD_0_AND_INSERT,HID_KEYPAD_PERIOD_AND_DELETE,KEY_F5,KEY_QUOTE,KEY_RESERVED,KEY_F6 },
	{ KEY_PRINTSCREEN,(KeyboardKeycode)0xF5,(KeyboardKeycode)0xF0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_7,KEY_F11,KEY_RESERVED,KEY_F12,KEY_RESERVED,KEY_F10,KEY_0,KEY_9,KEY_8 },
	{ (KeyboardKeycode)0xF9,KEY_RESERVED,KEY_LEFT_CTRL,KEY_TILDE,KEY_F1,KEY_F2,KEY_5,KEY_6,KEY_DELETE,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_F9,KEY_MINUS,KEY_F8,KEY_EQUAL }
};

ConsumerKeycode mediakeys[9]{HID_CONSUMER_VOLUME_DECREMENT, HID_CONSUMER_MUTE, HID_CONSUMER_VOLUME_INCREMENT, MEDIA_PREVIOUS, MEDIA_PLAY_PAUSE, MEDIA_NEXT, /*Opens media player*/ HID_CONSUMER_AL_AUDIO_PLAYER,  };
TimedAction checkBatteryAction = TimedAction();

FunctionType funcType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
FunctionType oldFuncType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
OperateMode operateMode = NONE;
OperateMode oldOperateMode = NONE;
uint8_t leds, oldLeds;
bool processKeys = false;
struct Settings {
	byte btaddr1[6];
	byte btaddr2[6];
	byte btaddr3[6];
	uint16_t mouseaccel[6];
};
Settings settings;
struct Key {
	byte code;
	byte state;
	byte num;
};

Key rkey;
Key *rkeys[6];
bool GetBit(byte thebyte, int position)
{
	return (1 == ((thebyte >> position) & 1));
}

void doMouseStuff()
{
	/*MouseData data = ps2_mouse.readData();
	leftMouseButton = GetBit(data.status, 0);
	rightMouseButton = GetBit(data.status, 1);
	Serial.print("\tx=");
	Serial.print(data.position.x);
	Serial.print("\ty=");
	Serial.print(data.position.y *-1);
	Serial.print("\ts=");
	Serial.print(data.status, BIN);
	Serial.println();
	if (leftMouseButton) {
		if (!Mouse.isPressed(MOUSE_LEFT)) {
			Mouse.press(MOUSE_LEFT);
		}
	}
	else {
		if (Mouse.isPressed(MOUSE_LEFT)) {
			Mouse.release(MOUSE_LEFT);
		}
	}

	if (rightMouseButton) {
		if (!Mouse.isPressed(MOUSE_RIGHT)) {
			Mouse.press(MOUSE_RIGHT);
		}
	}
	else {
		if (Mouse.isPressed(MOUSE_RIGHT)) {
			Mouse.release(MOUSE_RIGHT);
		}
	}
	if (operateMode == BLUETOOTH) {
		bt->mouseMove(data.position.x, data.position.y);
	}
	else if (operateMode == USB) {
		BootMouse.move(data.position.x, data.position.y);
	}
	*/
}

void checkForLeds() {
	leds = BootKeyboard.getLeds();
	if (leds != oldLeds) {
		Serial1.write('l');
		Serial1.write(leds);
	}
	oldLeds = leds;
}

void checkBattery() {
	uint16_t batVolt = analogReadVoltage(BATPIN) * 100;
	Serial1.write('b');
	Serial1.write(batVolt);
}
void setup() {
	Serial.begin(115200);
	Serial.println(F("Initializing HIDProcessor"));
	checkBatteryAction = TimedAction(1000, *checkBattery);
	Serial.println(F("Initializing HID"));
	BootMouse.begin();
	BootKeyboard.begin();
	Consumer.begin();
	Serial.println(F("HID Initilized"));
	Serial.println(F("Inititing BTModule"));
	//bt->begin(BP_MODE_HID, BP_HID_COMBO);
	Serial.println(F("BTModule initialized"));
	Serial.println(F("Initializing DuckyParser"));
	if (duckyParser.init(SD_ENABLE)) {
		Serial.println(F("DuckyParser Initialized"));
	}
	else {
		Serial.println(F("DuckyParser failed to Initialized"));
	}
	Serial.println(F("Initializing link to Main Processor"));
	Serial1.begin(115200); 
	Serial.println(F("Main processor link initialized"));
	Serial.println(F("Initializing Done, Type away"));
}

void pressKeyboardKey(KeyboardKeycode key) {
	if (operateMode == BLUETOOTH) {
		//bt->keyboardPress(key, BP_MOD_NOMOD);
	}
	else if (operateMode == USB) {
		BootKeyboard.press(key);
	}
}
void releaseKeyboardKey(KeyboardKeycode key) {
	if (operateMode == BLUETOOTH) {
	}
	else if (operateMode == USB) {
		BootKeyboard.release(key);
	}
}
void writeKeyboardKey(KeyboardKeycode key) {
	if (operateMode == BLUETOOTH) {
	}
	else if (operateMode == USB) {
		BootKeyboard.write(key);
	}
}

void ConsumerWrite(ConsumerKeycode key) {
	if (operateMode == BLUETOOTH) {
		//bt->sendConsumerCommand(lowByte(key), highByte(key));
		//bt->keyRelease();
	}
	else if (operateMode == USB) {
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
		switch (MEDIA) {
		case 0:
			break;
		case 1:
			break;
		}
		return;
	}
	if (funcType == ONEPRESS) {
		switch (key)
		{
		case KEY_UP_ARROW:
			//Consumer.write(LAPTOP_BRIGHTNESS_UP);
			break;
		case KEY_DOWN_ARROW:
			//Consumer.write(LAPTOP_BRIGHTNESS_DOWN);
			break;
		default:
			break;
		}
		return;
	}
	// From here on we know it is not a special key so we can use some special things without directly pressing the button
	pressKeyboardKey((KeyboardKeycode)key);
}
void processReleasedKey(byte key) {
	if (key == 0xF0) {
		funcType = oldFuncType;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		switch (funcType) {
			case MEDIA:
				ConsumerWrite(mediakeys[func]);
				break;
			case ONEPRESS:
				if (operateMode == USB) { // I dont think bluetooth is gonna handle this
					duckyParser.ExecDucky(func); // This is gonna be on release
				}
				break;
		}
		return;
	}
	BootKeyboard.release((KeyboardKeycode)key);
}
void checkKeys() {
	if (processKeys)
	{
		for (int i = 0; i<6; i++)   // Scan the whole key list.
		{
			switch (rkeys[i]->state) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
			case PRESSED:
				Serial.print("Key: ");
				Serial.print((uint8_t)rkeys[i]->code);
				Serial.println("\tpressed");
				processPressedKey(rkeys[i]->code);
				break;
			case RELEASED:
				Serial.print("Key: ");
				Serial.print((uint8_t)rkeys[i]->code);
				Serial.println("\treleased");
				processReleasedKey(rkeys[i]->code);
				break;
			case IDLE:
			case HOLD:
				break;
			}
		}
		processKeys = false;
	}
}
void checkSerial() {
	while(Serial1.available()) { // Read
		char command = (char)Serial1.read();
		switch (command)
		{
		case 'k': // We recieved keys
			Serial1_readAnything(rkey);
			*rkeys[0] = rkey;
			Serial1_readAnything(rkey);
			*rkeys[1] = rkey;
			Serial1_readAnything(rkey);
			*rkeys[2] = rkey;
			Serial1_readAnything(rkey);
			*rkeys[3] = rkey;
			Serial1_readAnything(rkey);
			*rkeys[4] = rkey;
			Serial1_readAnything(rkey);
			*rkeys[5] = rkey;
			processKeys = true;
			break;
		case 's': // We recieved settings
			Serial1_readAnything(settings);
			break;
		case 'f': // function mode
			funcType = (FunctionType)Serial.read();
			break;
		case 'm': // Mouse data
			MouseData data;
			Serial1_readAnything(data);
			Serial.print("\tx=");
			Serial.print(data.position.x);
			Serial.print("\ty=");
			Serial.print(data.position.y *-1);
			Serial.print("\ts=");
			Serial.print(data.status, BIN);
			Serial.println();
		default:
			break;
		}
	}
}
void loop() {
	if (analogReadVoltage(A1) == 0) {
		operateMode = OperateMode::CHARGING;
	}
	else if (analogReadVoltage(A1) > 3.0 && analogReadVoltage(A1) < 3.6) {
		operateMode = OperateMode::BLUETOOTH;
	}
	else if (analogReadVoltage(A1) > 4) {
		operateMode = OperateMode::USB;
		checkForLeds();
	}
	if (operateMode != oldOperateMode) {
		Serial1.write('o');
		Serial1.write(operateMode);
	}
	oldOperateMode = operateMode;
	checkBatteryAction.check();
	checkKeys();
}
