#include <SdFs.h>
#include "Variables.h"
#include "SettingsHelper.h"
#include "Helpers.h"
#include <ClickEncoder.h>
#include "Helpers.h"
#include "Config.h"
#include "OledFunctions.h"
#include <PS2Mouse.h>
#include <USBHID.h>
#include "usb_mass.h"
#include <KeyboardCodes.h>
#include <ConsumerKeyCodes.h>
#include "HIDFunctions.h"
#include "Variables.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "DuckyReadWriter.h"
#include <Adafruit_MCP23008.h>
#include <BPLib.h>
#include <Adafruit_MCP23017.h>
#include <Wire.h>
#include "I2CMatrix.h"
#include "Key.h"
#include <TimedAction.h>
#include <USBComposite.h>
//Keyboard gray = gnd blue = vcc 
//Mouse gray = gnd blue = vcc purple = data white = clock


HardwareTimer timerOne(1);

Adafruit_SSD1306 main_oled(OLED_DC, OLED_RESET, OLED_CS);

ClickEncoder *encoder = new ClickEncoder(ENC_DT, ENC_CLK, ENC_SW, 1);
Adafruit_MCP23008 expender = Adafruit_MCP23008();
bool expenderinterupted = false;

int16_t lastEncPos, currentEncPos;
enum KeyboardLeds : uint8_t {
	LED_NUM_LOCK = (1 << 0),
	LED_CAPS_LOCK = (1 << 1),
	LED_SCROLL_LOCK = (1 << 2)
};
Adafruit_MCP23017 *rowChip = new Adafruit_MCP23017();
Adafruit_MCP23017 *colChip = new Adafruit_MCP23017();
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
uint16_t programmingKeys[9]{ HID_KEYBOARD_HOME, 0x00, HID_KEYBOARD_END, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

PS2Mouse ps2_mouse(CLOCK_PIN, DATA_PIN);
bool leftMouseButton = 0;
bool rightMouseButton = 0;
bool mouseResetSend = false;
bool capsState = false, numState = false, scrollState = false;
TimedAction doMouseStuffAction = TimedAction();
TimedAction checkBatteryAction = TimedAction();
I2CMatrixClass matrix = I2CMatrixClass();
float batteryvoltage;
FunctionType funcType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
FunctionType oldFuncType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys

#define FORCEOPMODE
OperateMode operateMode = CABLE;

SdFs SD;
uint32_t cardSize = 0;


BPLib *bt;
bool bluetoothConnected = false;


Settings settings;

uint8_t menuindex = 0; // 0 = showing func type

usb_dev_state oldUsbState = USB_UNCONNECTED;

#if HID_ENABLED

const uint8_t reportDescription[] = {
	HID_MOUSE_REPORT_DESCRIPTOR(HID_MOUSE_REPORT_ID),
	HID_KEYBOARD_REPORT_DESCRIPTOR(HID_KEYBOARD_REPORT_ID),
	HID_CONSUMER_REPORT_DESCRIPTOR(HID_CONSUMER_REPORT_ID)
};
#endif

USBHID HID;
HIDKeyboard Keyboard(HID);
HIDMouse Mouse(HID);
HIDConsumer Consumer(HID);
USBMassStorage MassStorage;
bool massStarted = false;
bool sdInitialized = false;


uint8 leds = 0;
uint8 oldLeds = 0;

bool GetBit(byte thebyte, int position)
{
	return (1 == ((thebyte >> position) & 1));
}
void printBits(int var, bool newLine) {
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
	if (newLine)
		Serial.println();
}

void doMouseStuff()
{
#if MOUSE_ENABLED
	digitalWrite(PA1, HIGH);
	MouseData data = ps2_mouse.readData();
	digitalWrite(PA1, LOW);
	leftMouseButton = GetBit(data.status, 0);
	rightMouseButton = GetBit(data.status, 1);
#if DEBUGMOUSE
	Serial.print(F("status:"));
	printBits(data.status, false);
	Serial.print(F("\tX:"));
	Serial.print(data.position.x);
	Serial.print(F("\tY:"));
	Serial.println(data.position.y);
#endif
#if HID_ENABLED || BT_ENABLED
	if (funcType != FNKEYS) {
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
	}
	else
	{
		if (leftMouseButton || rightMouseButton) {
			PressMouse(MOUSE_MIDDLE);
		}
		else {
			ReleaseMouse(MOUSE_MIDDLE);
		}
	}
	if (data.position.x != 0 || data.position.y != 0) {
		mouseResetSend = false;
		if (funcType != FNKEYS)
		{
			MoveMouse(round(data.position.x * settings.mouseaccel[0]), round(data.position.y * settings.mouseaccel[1] * -1));
		}
		else
		{
			if (abs(data.position.x) > abs(data.position.y))
			{
				moveScrollWheel(data.position.x > 0 ? 1 : -1, 0);
			}
			else
			{
				moveScrollWheel(0, data.position.y > 0 ? 1 : -1);
			}
		}

	}
	else if (mouseResetSend == false) {
		MoveMouse(0, 0);
		moveScrollWheel(0, 0);
		mouseResetSend = true;
	}
#endif
#endif
}



void checkBattery() {
	//batteryvoltage = analogReadVoltage(BATPIN) * 2; // Cutting voltage in half with voltage divider to get a range of 1.5 to 2.1 volt. > 3,3 volt on ADC will kill the STM32

	digitalWrite(PA3, HIGH);
	redrawhud();
	digitalWrite(PA3, LOW);
}

void checkForLeds() {
	if (operateMode == CABLE && USBLIB->state == USB_SUSPENDED) {
		leds = Keyboard.getLEDs();
	}
	else {
		leds = bt->keyboardGetLeds();
	}
	if (leds != oldLeds) {
		printBits(leds, true);
		if (leds & LED_CAPS_LOCK && !capsState) {
			rowChip->digitalWrite(15, LOW);
			capsState = true;
		}
		else if (capsState) {
			rowChip->digitalWrite(15, HIGH);
			capsState = false;
		}
		if (leds & LED_NUM_LOCK && !numState) {
			rowChip->digitalWrite(14, LOW);
			numState = true;
		}
		else if (numState) {
			rowChip->digitalWrite(14, HIGH);
			numState = false;
		}
		if (leds & LED_SCROLL_LOCK && !scrollState) {
			rowChip->digitalWrite(13, LOW);
			scrollState = true;
		}
		else if (scrollState) {
			rowChip->digitalWrite(13, HIGH);
			scrollState = false;
		}
		oldLeds = leds;
	}
}

void mcpInt()
{
	expenderinterupted = true;
}

void timerIsr() {
	encoder->service();
}

// Mass storage part
bool massSDwrite(const uint8_t *writebuff, uint32_t startSector, uint16_t numSectors) {
	return SD.card()->writeSectors(startSector, writebuff, numSectors);
}

bool massSDread(uint8_t *readbuff, uint32_t startSector, uint16_t numSectors) {
	return SD.card()->readSectors(startSector, readbuff, numSectors);
}

void initSDReader() {
	Serial.println(F("Starting MassStorage"));
	MassStorage.setDriveData(0, cardSize, massSDread, massSDwrite);
	massStarted = true;
}

void setup() {
	Serial.begin(115200);
	pinMode(INIT_LED, OUTPUT);
	delay(300);
	Serial.println(F("Starting custom keyboard"));
	Serial.println(F("Initializing Wire"));
	Wire.begin();
	Wire.setClock(400000);
	//Wire.setClock(1700000); // 1.7 MHz
	Serial.println(F("Initializing SD Card "));
	if (SD.begin(SD_CONFIG_STARTUP)) {
		uint32_t cardSize = SD.card()->sectorCount();
		Serial.println(F("SD Card initialized"));
		sdInitialized = true;
	}
	else
	{
		Serial.print(F("SD ERROR: 0X"));
		Serial.print(SD.sdErrorCode(), HEX);
		Serial.print(F(",0X"));
		Serial.println(SD.sdErrorData(), HEX);
		sdInitialized = false;
	}
	//initVars();
	loadSettings();
	Serial.println(F("Initializing keyboard wire things"));
	delay(50);
	rowChip->begin(B000);
	if (!rowChip->isConnected()) {
		Serial.println(F("Rowchip not found. Check wiring"));
		while (1);
	}
	colChip->begin(B100);
	if (!colChip->isConnected()) {
		Serial.println(F("Colchip not found. Check wiring"));
		while (1);
	}
	expender.begin(B001);
	if (!expender.isConnected()) {
		Serial.println(F("MCP Expender not found. Check wiring"));
		while (1);
	}

	pinMode(MCPINT, INPUT);
	expender.write8(MCP23008_GPPU, 0b00111111); // All inputs to input_pullup
	expender.write8(MCP23008_IPOL, 0b00111111); // Set pins to interupt when changed
	expender.write8(MCP23008_INTF, 0b00111111); // Set interupt flags on the inputs
	expender.write8(MCP23008_GPINTEN, 0b00111111); // Set interupt flags on the inputs
												  // expender.write8(MCP23008_GPPU, 0xff); // All inputs to input_pullup
												  // expender.write8(MCP23008_INTF, 0xff); // Set interupt flags on the inputs
												  // expender.write8(MCP23008_GPINTEN, 0xff); // Set interupt flags on the inputs
	expender.write8(MCP23008_INTCON, 0x00); // Set pins to interupt when changed
	byte IOCON = expender.read8(MCP23008_IOCON);
	IOCON &= ~(1 << 2); // Sets ODR in IOCON to Active Driver output
	IOCON &= ~(1 << 1); // Sets INTPOL to Active LOW
	expender.write8(MCP23008_IOCON, IOCON);
	expender.pinMode(BT_CONNECTED_LED, OUTPUT);
	expender.pinMode(INIT_LED, OUTPUT);
	attachInterrupt(MCPINT, mcpInt, FALLING); // Sets interupt to detect LOW to HIGH
	Serial.println(F("MCP Expender Initialized"));

	delay(10);
	Serial.println(F("keyboard wire things initialized"));
	delay(10);

#if MOUSE_ENABLED
	Serial.println(F("Initializing PS2 Mouse"));
	ps2_mouse.initialize(); // If it is not connected DO NOT CALL ANY PS2 MOUSE FUNCTION
	doMouseStuffAction = TimedAction(10, &doMouseStuff);
	Serial.println(F("PS2 Mouse Initialized"));
	delay(10);
#endif // MOUSE_ENABLED

#if OLED_ENABLED
	Serial.println(F("Initializing Oled"));
	main_oled.begin();
	main_oled.display();
	delay(1000);
	main_oled.clearDisplay();
	main_oled.println(F("Oled Initialized"));
	main_oled.display();
	delay(1000);
	main_oled.clearDisplay();
	main_oled.display();
	//fullupdateoled();
	checkBatteryAction = TimedAction(1000 * 5, &checkBattery);
	Serial.println(F("Oled Initialized"));
	delay(10);
#endif

#if BT_ENABLED
	Serial.println(F("Initializing BT Chip"));
	Serial2.begin(115200);
	delay(10);
	bt = new BPLib(Serial2);
	// bt->begin(BP_MODE_HID, BP_HID_COMBO); // TODO: Change this back when I know why it is misbehaving
	// bt->setStatusString(BT_PREFIX);
	// //bt->begin(BP_MODE_SPP, BP_SPP_SPP); // Serial
	// if (settings.bluetoothAutoConnect == 0)
	// {
	// 	bt->reconnect();
	// }
	// else {
	// 	BluetoothAddress bt_address = settings.bt_addressess[settings.bluetoothAutoConnect];
	// 	Serial.print(F("Connecting to host "));
	// 	Serial.print(bt_address.name);
	// 	Serial.print(F(" with address "));
	// 	Serial.println(bt_address.address);
	// 	bt->reconnect(bt_address.address);
	// }
	Serial.println(F("BT Chip Initialized"));
	delay(10);
#endif
#if KEYBOARD_ENABLED
	Serial.println(F("Initializing Matrix"));
	delay(50);
	matrix.init(makeKeymap(keys), rowChip, colChip, ROWS, COLS);
	Serial.println(F("Starting Matrix"));
	digitalWrite(INIT_LED, HIGH);
	delay(2);
	matrix.begin();
	delay(2);
	digitalWrite(INIT_LED, LOW);
	Serial.println(F("Matrix started"));
	rowChip->pinMode(13, OUTPUT);
	rowChip->pinMode(14, OUTPUT);
	rowChip->pinMode(15, OUTPUT);
	rowChip->digitalWrite(13, HIGH);
	rowChip->digitalWrite(14, HIGH);
	rowChip->digitalWrite(15, HIGH);

	matrix.setDebounceTime(1);
	delay(10);
#endif
#if ENCODER_ENABLED
	Serial.println(F("Initializing Encoder"));
	delay(10);
	encoder->setAccelerationEnabled(false);
	encoder->setDoubleClickEnabled(true);
	timerOne.pause();
	timerOne.setPeriod(1000);
	// Set up an interrupt on channel 1
	timerOne.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	timerOne.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
	timerOne.attachCompare1Interrupt(timerIsr);
	timerOne.refresh(); // Refresh the timer's count, prescale, and overflow
	timerOne.resume(); // Start the timer counting
	lastEncPos = 0;
	delay(10);
	Serial.println(F("Encoder initialized"));
#endif // ENCODER_ENABLED

#if HID_ENABLED
	Serial.println(F("Initializing HID"));
	delay(100);
	USBComposite.end();
	USBComposite.clear();
	USBComposite.setProductString("Adv. Keyboard");
	USBComposite.setProductId(0x33);
	HID.setReportDescriptor(reportDescription, sizeof(reportDescription));
	HID.registerComponent();
	Keyboard.begin(); // Enables leds
	Keyboard.setKeyboardMode(RAW_HID);
	Serial.println(F("HID Initialized"));
	delay(10);
#endif
#if MASS_ENABLED
	initSDReader();
	MassStorage.registerComponent(); // Register MassStorage
#endif
	USBComposite.begin();	// Begins USBComposite with HID and (if enabled) MassStorage
	expenderinterupted = true; // Force update of expender
	//Serial.println(inputScreen(12,"HEX", HEXCHARS));
	//Serial.println(inputScreen(12,"lowercase", LOWER_ALPHABET));
	//Serial.println(inputScreen(12,"UPPERCASE", UPPER_ALPHABET));
	//Serial.println(inputScreen(12,"NUMBERS.-", NUMBERS));
	fullupdateoled();
	Serial.println(F("Initializing Done, Keyboard is ready"));
	delay(100);
}

void handleEncoder() {
	currentEncPos = encoder->getValue();
	if (currentEncPos != lastEncPos) {
		lastEncPos = currentEncPos;
		Serial.print(F("Encoder Value: "));
		Serial.println(currentEncPos);
	}
	ClickEncoder::Button b = encoder->getButton();
	if (b != ClickEncoder::Open) {
		Serial.print(F("Button: "));
		switch (b) {
			VERBOSECASE(ClickEncoder::Pressed)
				VERBOSECASE(ClickEncoder::Held)
				VERBOSECASE(ClickEncoder::Released)
				VERBOSECASE(ClickEncoder::Clicked)
				VERBOSECASE(ClickEncoder::DoubleClicked)
		}
	}
}

void processPressedKey(byte key) {
#if DEBUGKEYBOARD
	Serial.print(F("Processing pressed key: "));
	Serial.println(key);
#endif
	if (key == 0xF0) {
		oldFuncType = funcType;
		funcType = FNKEYS;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		switch (funcType) {
		case MEDIA:
			pressConsumerKey(mediakeys[func]);
			break;
		case FNKEYS:
			break;
		}
		return;
	}
	if (funcType == FNKEYS) {
		switch (key)
		{
		case KEY_UP:
			writeConsumerKey(CONSUMER_BRIGHTNESS_UP);
			break;
		case KEY_DOWN:
			writeConsumerKey(CONSUMER_BRIGHTNESS_DOWN);
			break;
		case KEY_LEFT:
			writeKeyboardKey(HID_KEYBOARD_HOME);
			// PressMouse(MOUSE_BACK);
			// ReleaseMouse(MOUSE_BACK);
			break;
		case KEY_RIGHT:
			writeKeyboardKey(HID_KEYBOARD_END);
			// PressMouse(MOUSE_FORWARDS);
			// ReleaseMouse(MOUSE_FORWARDS);
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
#if DEBUGKEYBOARD
	Serial.print(F("Processing released key: "));
	Serial.println(key);
#endif
	if (key == 0xF0) {
		funcType = oldFuncType;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		switch (funcType) {
		case MEDIA:
			releaseConsumerKey();
			break;
		case FNKEYS:

			break;
		}
		return;
	}

	releaseKeyboardKey(key);
}

void checkKeys() {
	if (matrix.getKeys())
	{
		for (int i = 0; i < LIST_MAX; i++)   // Scan the whole key list.
		{
			if (matrix.key[i].stateChanged)   // Only find keys that have changed state.
			{
				switch (matrix.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
				case PRESSED:
#if DEBUG
					Serial.print(F("Key: "));
					Serial.print((uint8_t)matrix.key[i].kcode);
					Serial.println(F("\tpressed"));
#endif
#if HID_ENABLED || BT_ENABLED
					processPressedKey(matrix.key[i].kcode);
#endif
					break;
				case RELEASED:
#if DEBUG
					Serial.print(F("Key: "));
					Serial.print((uint8_t)matrix.key[i].kcode);
					Serial.println(F("\treleased"));
#endif
#if HID_ENABLED || BT_ENABLED
					processReleasedKey(matrix.key[i].kcode);
#endif
					break;
				case HOLD:
					// #if DEBUG
					// 	Serial.print(F("Key: "));
					// 	Serial.print((uint8_t)matrix.key[i].kcode);
					// 	Serial.println(F("\tholded"));
					// #endif
				case IDLE:
					break;
				}
			}
		}
	}
}

void loop() {
	if (expenderinterupted)
	{
		Serial.println(F("Expender interupted"));
		byte gpio = expender.readGPIO();
		printBits(gpio, true);
#ifndef FORCEOPMODE
		if (bitRead(gpio, USBMODEPIN)) {
			operateMode = CABLE;
		}
		else if (bitRead(gpio, BTMODEPIN))
		{
			operateMode = BLUETOOTH;
		}
		else {
			operateMode = CHARGING;
		}
#endif
#if MASS_ENABLED
		if (bitRead(gpio, SDUSBPIN))
		{
			if (!massStarted && sdInitialized)
			{
				initSDReader();
			}
		}
		else if (!bitRead(gpio, SDUSBPIN))
		{
			// if (massStarted) {
			// 	MassStorage.setDriveData(0, 0, NULL);
			// 	massStarted = false;
			// }
		}
#endif
		expenderinterupted = false;
	}
	if (massStarted)
	{
		MassStorage.loop();
	}
	while (Serial2.available() > 0) {
		Serial.write(Serial2.read());
	}
	while (Serial.available() > 0) {
		Serial2.write(Serial.read());
	}

	checkBatteryAction.check();
#if MOUSE_ENABLED
	doMouseStuffAction.check();
#endif
#if HID_ENABLED
	if (USBLIB->state != oldUsbState)
	{
		Serial.print(F("USBSTATE: "));
		switch (USBLIB->state)
		{
			VERBOSECASE(USB_UNCONNECTED)
				VERBOSECASE(USB_ADDRESSED)
				VERBOSECASE(USB_ATTACHED)
				VERBOSECASE(USB_POWERED)
				VERBOSECASE(USB_SUSPENDED)
				VERBOSECASE(USB_CONFIGURED)
		}
		oldUsbState = USBLIB->state;
	}
#endif
#if KEYBOARD_ENABLED
#if HID_ENABLED
	checkForLeds();
#endif
	checkKeys();
#endif

#if ENCODER_ENABLED
	handleEncoder();
#endif
}
