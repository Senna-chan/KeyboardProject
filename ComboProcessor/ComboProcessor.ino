#include <SPI.h>
#include "KeyReport.h"
#include <Keypad.h>
#include "KeyboardHelpers.h"
#include "Config.h"
#include "Variables.h"
#include "Helpers.h"
#include "MacroHelper.h"
#include <BPLib.h>
#include <i2cEncoderLibV2.h>
#include <SdFs.h>
#include "SettingsHelper.h"
#include "OledFunctions.h"
#include <PS2Mouse.h>
#include <USBHID.h>
#include "usb_mass.h"
#include <KeyboardCodes.h>
#include <ConsumerKeyCodes.h>
#include "HIDFunctions.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "DuckyReadWriter.h"
#include <Adafruit_MCP23008.h>
#include <Adafruit_MCP23017.h>
#include <Wire.h>
#include "Key.h"
#include <TimedAction.h>
#include <USBComposite.h>



const byte ROWS = 8;
const byte COLS = 16;

byte rowPins[ROWS] = { PD15,PD14,PD4,PD5,PD13,PD7,PB13,PB12}; //connect to the row pinouts of the keypad
byte colPins[COLS] = { PD10, PD9, PD8, PE15, PE14,PE13,PE12,PE11,PE10,PE9,PE8,PE7,PD1,PD2,PD15,PD14 }; //connect to the column pinouts of the keypad



//Keyboard gray = gnd blue = vcc 
//Mouse gray = gnd blue = vcc purple = data white = clock



HardwareTimer timerOne(1);

Adafruit_MCP23008 expender = Adafruit_MCP23008();
bool expenderinterupted = false;
bool encoderInterupted = false;

int16_t lastEncPos, currentEncPos;
enum KeyboardLeds : uint8_t {
	LED_NUM_LOCK = (1 << 0),
	LED_CAPS_LOCK = (1 << 1),
	LED_SCROLL_LOCK = (1 << 2)
};

uint8_t keys[ROWS][COLS] =
{
	{ KEY_LEFT_ALT,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_B,KEY_N,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEYPAD_SUBTRACT,KEY_SPACE,KEY_SLASH,KEY_RESERVED,KEY_RESERVED },
	{ KEY_RESERVED,KEY_RIGHT,KEY_RIGHT_CONTROL,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_M,KEY_NUM_LOCK,KEY_PAUSE,KEYPAD_DIVIDE,KEYPAD_MULTIPLY,KEY_ENTER,KEY_RESERVED,KEY_PERIOD,KEY_COMMA },
	{ KEY_RESERVED,(KeyboardKeycode)0xF7,(KeyboardKeycode)0xF3,KEY_A,KEY_S,KEY_D,KEY_F,KEY_J,KEYPAD_1,KEY_ENTER,KEYPAD_2,KEYPAD_3,KEY_BACKSLASH,KEY_SEMICOLON,KEY_L,KEY_K },
	{ KEY_RESERVED,KEY_LEFT_SHIFT,(KeyboardKeycode)0xF2,KEY_TAB,KEY_CAPS_LOCK,KEY_F3,KEY_T,KEY_Y,KEYPAD_4,KEY_RESERVED,KEYPAD_5,KEYPAD_6, KEY_BACKSPACE ,KEY_LEFT_BRACE,KEY_F7,KEY_RIGHT_BRACE },
	{ KEY_SCROLL_LOCK,(KeyboardKeycode)0xF6,(KeyboardKeycode)0xF1,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_U,KEYPAD_7,KEYPAD_ADD,KEYPAD_8,KEYPAD_9,KEY_RESERVED,KEY_P,KEY_O,KEY_I },
	{ KEY_LEFT_ALT,(KeyboardKeycode)0xF8,(KeyboardKeycode)0xF4,KEY_ESCAPE,KEY_RESERVED,KEY_F4,KEY_G,KEY_H,KEY_RESERVED,KEY_UP,KEYPAD_0,KEYPAD_DOT,KEY_F5,KEY_QUOTE,KEY_RESERVED,KEY_F6 },
	{ KEY_PRINTSCREEN,(KeyboardKeycode)0xF5,(KeyboardKeycode)0xF0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_7,KEY_F11,KEY_RESERVED,KEY_F12,KEY_RESERVED,KEY_F10,KEY_0,KEY_9,KEY_8 },
	{ (KeyboardKeycode)0xF9,KEY_RESERVED,KEY_LEFT_CONTROL,KEY_TILDE,KEY_F1,KEY_F2,KEY_5,KEY_6,  76  /*DELETE*/,KEY_RESERVED,KEY_RESERVED,KEY_RESERVED,KEY_F9,KEY_MINUS,KEY_F8,KEY_EQUAL }
};
uint16_t mediakeys[9]{ HID_CONSUMER_VOLUME_DECREMENT, HID_CONSUMER_MUTE, HID_CONSUMER_VOLUME_INCREMENT, MEDIA_PREVIOUS, MEDIA_PLAY_PAUSE, MEDIA_NEXT, /*Opens media player*/ HID_CONSUMER_AL_AUDIO_PLAYER, };
uint16_t programmingKeys[9]{ KEY_HOME, 0x00, KEY_END, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

PS2Mouse ps2_mouse(CLOCK_PIN, DATA_PIN);
bool leftMouseButton = 0;
bool rightMouseButton = 0;
bool mouseResetSend = false;
bool capsState = false, numState = false, scrollState = false;
TimedAction doMouseStuffAction = TimedAction();
TimedAction checkBatteryAction = TimedAction();
TimedAction secondsTickAction = TimedAction();
Keypad matrix = Keypad();
float batteryvoltage;
String funcType = "MEDIA";
String oldFuncType = "STARTUP";
bool fnKeys = false;

bool forceOperateMode = true;
OperateMode operateMode = CABLE;

SdFs SD;
uint32_t cardSize = 0;


BPLib *bt = new BPLib(Serial2);
i2cEncoderLibV2 *encoder = new i2cEncoderLibV2(ENC_ADDR);
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
USBCompositeSerial CompositeSerial;
bool massStarted = false;
bool sdInitialized = false;


uint8 leds = 0;
uint8 oldLeds = 255;

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




void checkBattery() {
	batteryvoltage = analogReadVoltage(BATPIN) * 2; // Cutting voltage in half with voltage divider to get a range of 1.5 to 2.1 volt. > 3,3 volt on ADC will kill the STM32
	batteryvoltage = round(batteryvoltage * 10);
	batteryvoltage = batteryvoltage / 10; // Getting one decimal after the comma i hope
	redrawhud();
}


void mcpInt() { expenderinterupted = true; }
void handleEncoderInt() { encoderInterupted = true; }
bool reinitSD = false;
void handleSDDetectInt()
{
	if (!digitalRead(SD_CD)) {
		SD.end(); // We need to end the sd card because of reasons
		if (massStarted) {
			MassStorage.setDriveData(0, 0, NULL, NULL);
			massStarted = false;
		}
	}
	else reinitSD = true;
}
#pragma region massStorage region
// Mass storage part
bool massSDwrite(const uint8_t *writebuff, uint32_t startSector, uint16_t numSectors) {
	bool result = SD.card()->writeSectors(startSector, writebuff, numSectors);
	if (!result) Serial.println("Failed to write");
	return result;
}

bool massSDread(uint8_t *readbuff, uint32_t startSector, uint16_t numSectors) {
	bool result = SD.card()->readSectors(startSector, readbuff, numSectors);
	if (!result) Serial.println("Failed to read");
	return result;
}

void initSDReader() {
	MassStorage.setDriveData(0, cardSize, massSDread, massSDwrite);
	massStarted = true;
}
#pragma endregion 

void initSd(bool reinit = false)
{
	if (reinit)
	{
		delay(2000); // Delay because the detected line is going high a lot sooner then the sd card is inserted for reading
	}
	Serial.println(F("Initializing SD Card "));
	if (SD.begin(SD_CONFIG)) {
		cardSize = SD.card()->sectorCount();
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
	if (sdInitialized && reinit) // We need to save the stuff we may have edited
	{
		saveSettings();
		saveMacro();
		initSDReader();
	}
	else if (!reinit) { // We are in the booting sequence
		attachInterrupt(SD_CD, handleSDDetectInt, CHANGE);
		loadSettings();
	}
	reinitSD = false;
}

void secondTick() {
	seconds++;
	Serial.println(seconds);
	if (seconds - menuActivationTime > menuReturnSeconds && menuActive) {
#if DEBUG
		Serial.println(F("Timer of menu ran out"));
#endif
		menuActive = false;
		fulloledupdate();
	}
}// Write functions here that needs to happen every second(Counter is not precise tho)


#pragma region HIDActions
void processPressedKey(byte key) {
	if (key == 0xF0) {
		fnKeys = true;
		releaseAllKeyboardKeys();
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
		pressMacro(func);
	}
	if (fnKeys) {
		switch (key)
		{
		case KEY_UP:
			writeKeyboardKey(KEY_PAGE_UP);
			break;
		case KEY_DOWN:
			writeKeyboardKey(KEY_PAGE_DOWN);
			break;
		case KEY_LEFT:
			writeKeyboardKey(KEY_HOME);
			break;
		case KEY_RIGHT:
			writeKeyboardKey(KEY_END);
			break;
		case KEY_F1:
			writeConsumerKey(MEDIA_VOLUME_MUTE);
			break;
		case KEY_F2:
			writeConsumerKey(MEDIA_VOLUME_DOWN);
			break;
		case KEY_F3:
			writeConsumerKey(MEDIA_VOLUME_UP);
			break;
		case KEY_F5:
			writeConsumerKey(CONSUMER_BRIGHTNESS_DOWN);
			break;
		case KEY_F6:
			writeConsumerKey(CONSUMER_BRIGHTNESS_UP);
			break;
		case KEY_F10:
			writeConsumerKey(MEDIA_PLAY_PAUSE);
			break;
		case KEY_F11:
			writeConsumerKey(MEDIA_PREV);
			break;
		case KEY_F12:
			writeConsumerKey(MEDIA_NEXT);
			break;
		default:
			break;
		}
		return;
	}
	// From here on we know it is not a special key so we can just press it
	pressKeyboardKey(key);
}
void processReleasedKey(uint8_t key) {
	if (key == 0xF0) {
		fnKeys = false;
	}
	if (key >= 0xF1 && key <= 0xF9) {
		int func = key - 0xF1;
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
#if DEBUGKEYBOARD
					Serial.print(F("pressed "));
					Serial.println(KeyboardKeycode_string_table[(uint8_t)matrix.key[i].kcode]);
#endif
#if HID_ENABLED || BT_ENABLED
					processPressedKey(matrix.key[i].kcode);
#endif
					break;
				case RELEASED:
#if DEBUGKEYBOARD
					Serial.print(F("Released "));
					Serial.println(KeyboardKeycode_string_table[(uint8_t)matrix.key[i].kcode]);
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

void doMouseStuff()
{
#if MOUSE_ENABLED && (HID_ENABLED || BT_ENABLED)
	MouseData data = ps2_mouse.readData();
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
	if (!fnKeys) {
		if (leftMouseButton) {
			pressMouse(MOUSE_LEFT);
		}
		else {
			releaseMouse(MOUSE_LEFT);
		}

		if (rightMouseButton) {
			pressMouse(MOUSE_RIGHT);
		}
		else {
			releaseMouse(MOUSE_RIGHT);
		}
	}
	else
	{
		if (leftMouseButton || rightMouseButton) {
			pressMouse(MOUSE_MIDDLE);
		}
		else {
			releaseMouse(MOUSE_MIDDLE);
		}
	}
	if (data.position.x != 0 || data.position.y != 0) {
		mouseResetSend = false;
		if (!fnKeys)
		{
			moveMouse(round(data.position.x * currentConnectionConfig.mouseaccel[0]), round(data.position.y * currentConnectionConfig.mouseaccel[1] * -1));
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
		moveMouse(0, 0);
		moveScrollWheel(0, 0);
		mouseResetSend = true;
	}
#endif
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
			digitalWrite(CAPS_LED, LOW);
			capsState = true;
		}
		else if (capsState) {
			digitalWrite(CAPS_LED, HIGH);
			capsState = false;
		}
		if (leds & LED_NUM_LOCK && !numState) {
			digitalWrite(NUM_LED, LOW);
			numState = true;
		}
		else if (numState) {
			digitalWrite(NUM_LED, HIGH);
			numState = false;
		}
		if (leds & LED_SCROLL_LOCK && !scrollState) {
			digitalWrite(SCROLL_LED, LOW);
			scrollState = true;
		}
		else if (scrollState) {
			digitalWrite(SCROLL_LED, HIGH);
			scrollState = false;
		}
		oldLeds = leds;
	}
}
#pragma endregion

void setup() {

	Serial.begin(115200);

	pinMode(CAPS_LED, OUTPUT);
	pinMode(NUM_LED, OUTPUT);
	pinMode(SCROLL_LED, OUTPUT);
	digitalWrite(CAPS_LED, LOW);
	digitalWrite(NUM_LED, LOW);
	digitalWrite(SCROLL_LED, LOW);
#if DEBUG
	delay(200);
	Serial.println("DEBUG MODE ACTIVE. Prepare for a lot of output");
#endif
	pinMode(INIT_LED, OUTPUT);
	pinMode(BATPIN, INPUT_ANALOG);
	batteryvoltage = analogReadVoltage(BATPIN) * 2; // First Sample
	delay(300);
	Serial.println(F("Starting custom keyboard"));
	Serial.println(F("Initializing Wire"));
	Wire.begin();
	Wire.setClock(400000); // 400 kHz
	initVars();
	initSd();
	expender.begin(B001);
	if (!expender.isConnected()) {
		Serial.println(F("Expender not found. Check wiring"));
		while (1);
	}

	pinMode(MCPINT, INPUT);
	expender.write8(MCP23008_GPPU, 0xFF); // All inputs to input_pullup
	expender.write8(MCP23008_IPOL, 0xFF); // Set pins to interupt when changed
	expender.write8(MCP23008_INTF, 0xFF); // Set interupt flags on the inputs
	expender.write8(MCP23008_GPINTEN, 0xFF); // Set interupt flags on the inputs
	expender.write8(MCP23008_INTCON, 0x00); // Set pins to interupt when changed
	byte IOCON = expender.read8(MCP23008_IOCON);
	IOCON &= ~(1 << 2); // Sets ODR in IOCON to Active Driver output
	IOCON &= ~(1 << 1); // Sets INTPOL to Active LOW
	expender.write8(MCP23008_IOCON, IOCON);
	expender.pinMode(BT_CONNECTED_LED, OUTPUT);
	expender.pinMode(INIT_LED, OUTPUT);
	attachInterrupt(MCPINT, mcpInt, FALLING); // Sets interupt to detect HIGH to LOW
	Serial.println(F("MCP Expender Initialized"));
	secondsTickAction = TimedAction(1000, secondTick);
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
	main_oled.setTextColor(WHITE);
	main_oled.display();
	delay(1000);
	main_oled.clearDisplay();
	main_oled.println(F("Oled Initialized"));
	main_oled.display();
	delay(1000);
	main_oled.clearDisplay();
	main_oled.display();
	//fullupdateoled();
	Serial.println(F("Oled Initialized"));
	delay(10);
#endif
	checkBatteryAction = TimedAction(1000 * 5, &checkBattery);

#if BT_ENABLED
	Serial.println(F("Initializing BT Chip"));
	Serial2.begin(115200);
	delay(10);
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
	matrix.init(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

	matrix.setDebounceTime(1);
	delay(10);
#endif
#if ENCODER_ENABLED
	Serial.println(F("Initializing Encoder"));
	delay(10);
	pinMode(ENC_INT, INPUT_PULLUP);
	attachInterrupt(ENC_INT, handleEncoderInt, FALLING);
	// nvic_irq_set_priority(NVIC_I2C1_EV, 0);
	if (!encoder->begin(INT_DATA | WRAP_ENABLE | DIRE_RIGHT | IPUP_DISABLE | RMOD_X1 | STANDARD_ENCODER))
	{
		Serial.println("Encoder not found. Check wiring!");
		while (1);
	}
	else
	{
#if DEBUG
		Serial.println(F("Found the encoder"));
#endif
	}
	encoder->writeCounter((int32_t)0); /* Reset the counter value */
	encoder->writeMax((int32_t)30); /* Set the maximum threshold*/
	encoder->writeMin((int32_t)0); /* Set the minimum threshold */
	encoder->writeStep((int32_t)1); /* Set the step to 1*/
	encoder->writeInterruptConfig(0xff); /* Enable all the interrupt */
	encoder->writeAntibouncingPeriod(20);  /* Set an anti-bouncing of 200ms */
	encoder->writeDoublePushPeriod(50);  /*Set a period for the double push of 500ms */
	delay(10);
	Serial.println(F("Encoder initialized"));
#endif // ENCODER_ENABLED

#if HID_ENABLED
	Serial.println(F("Initializing HID"));
	delay(100);
	USBComposite.end();
	USBComposite.clear();
	USBComposite.setProductString("Adv. Keyboard");
	USBComposite.setProductId(0x34);
	HID.setReportDescriptor(reportDescription, sizeof(reportDescription));
	Keyboard.begin(); // Enables leds
	HID.registerComponent();
	Keyboard.setKeyboardMode(RAW_HID);
	Serial.println(F("HID Initialized"));
	delay(10);
#endif
#if MASS_ENABLED
	Serial.println(F("Initializing MassStorage"));
	initSDReader();
	MassStorage.registerComponent(); // Register MassStorage
	HID.setTXPacketSize(32); // We reached the hardware buffer size of 320 Serial(144) + HID(64) + MassStorage(128) == 336
	Serial.println(F("Initialized MassStorage"));
#endif
	CompositeSerial.registerComponent(); // Used for companion program communication
	USBComposite.setSerialString("Adv. Keyboard");
	if (!USBComposite.begin()) {	// Begins USBComposite with HID and (if enabled) MassStorage
		Serial.println(F("Failed to start USBComposite"));
		while (true) {
			gpio_toggle_bit(GPIOC, 13);
			delay(250);
		}
	}
	expenderinterupted = true; // Force update of expender
	// Serial.println(F("InputScreens"));
	// Serial.println(inputScreen(12,"HEX", HEXCHARS));
	//Serial.println(inputScreen(12,"lowercase", LOWER_ALPHABET));
	//Serial.println(inputScreen(12,"UPPERCASE", UPPER_ALPHABET));
	//Serial.println(inputScreen(12,"NUMBERS.-", NUMBERS));
	//Serial.println(F("Generating Macro"));
	//genMacro(0);
	fulloledupdate();
	Serial.println(F("Initializing Done, Keyboard is ready"));
	char key = matrix.getKey();
	if (key != 0x00) // For special conditions
	{
		switch(key)
		{
		case KEY_B:
			operateMode = BLUETOOTH;
			Serial.println(F("Forced mode to bluetooth"));
			break;
		case KEY_C:
			operateMode = CHARGING;
			Serial.println(F("Forced mode to charging"));
			break;
		case KEY_H:
			operateMode = CABLE;
			Serial.println(F("Forced mode to cable(HID)"));
			break;
		default:
			break;
		}
	}
	digitalWrite(CAPS_LED, HIGH);
	digitalWrite(NUM_LED, HIGH);
	digitalWrite(SCROLL_LED, HIGH);

}



void CompositeSerialAction() {
	Serial.print(F("CompositeSerial: "));
	String serialInput = CompositeSerial.readStringUntil('^');
	Serial.println(serialInput);
	String command = getStringPartByNr(serialInput, '|', 0);
	String parameter = getStringPartByNr(serialInput, '|', 1);
	if (command.equalsIgnoreCase("forceopmode"))
	{
		forceOperateMode = true;
		Serial.print("Old operateMode: ");
		switch (operateMode)
		{
			VERBOSECASE(BLUETOOTH);
			VERBOSECASE(CABLE);
			VERBOSECASE(CHARGING);
		}
		if (parameter.equalsIgnoreCase("cable"))
		{
			operateMode = CABLE;
		}
		else if (parameter.equalsIgnoreCase("bluetooth"))
		{
			operateMode = BLUETOOTH;
		}
		else if (parameter.equalsIgnoreCase("charging"))
		{
			operateMode = CHARGING;
		}
		else
		{
			forceOperateMode = false;
		}
		Serial.print(" new operateMode: ");
		switch (operateMode)
		{
			VERBOSECASELN(BLUETOOTH);
			VERBOSECASELN(CABLE);
			VERBOSECASELN(CHARGING);
		}
		redrawhud();
	}
	if (command.equalsIgnoreCase("switchfunctype"))
	{
	}
}

void loop() {
	if (massStarted)
	{
		MassStorage.loop();
	}
	secondsTickAction.check();
	if (CompositeSerial.available() > 0)
	{
		CompositeSerialAction();
	}
	if (expenderinterupted)
	{
		byte gpio = expender.readGPIO();
#if DEBUG
		Serial.print(F("Expender interupted "));
		printBits(gpio, true);
#endif
		if (!forceOperateMode) {
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

		 	switch (operateMode)
		 	{
		 		VERBOSECASELN(BLUETOOTH);
		 		VERBOSECASELN(CABLE);
		 		VERBOSECASELN(CHARGING);
		 	}
		}
		// #if MASS_ENABLED
		// 		if (bitRead(gpio, SDUSBPIN))
		// 		{
		// 			if (!massStarted && sdInitialized)
		// 			{
		// 				initSDReader();
		// 			}
		// 		}
		// 		else if (!bitRead(gpio, SDUSBPIN))
		// 		{
		// 			// if (massStarted) {
		// 			// 	MassStorage.setDriveData(0, 0, NULL);
		// 			// 	massStarted = false;
		// 			// }
		// 		}
		// #endif
		expenderinterupted = false;
	}

	checkBatteryAction.check();

	if (reinitSD)
	{
		Serial.println("SD Card was removed and is now connected");
		initSd(true); // Restart SD card
	}

	while (Serial.available() > 0)
	{
		Serial2.write(Serial.read());
	}
	if (bt->loop())
	{
		String btEvent = bt->getEvent();
		if (btEvent.startsWith("CONNECT"))
		{
			bluetoothConnected = true;
		}
		else if (btEvent.startsWith("DISCONNECT"))
		{
			bluetoothConnected = false;
		}
	}

	if (operateMode != CHARGING) {
#if MOUSE_ENABLED
		doMouseStuffAction.check();
#endif
#if ENCODER_ENABLED
		if (encoderInterupted) {
			handleEncoder();
			if (lastEncPos != currentEncPos)
			{

				if (settingsActive) {
					if (encoder->increased())
					{
						if (settingsindex != 1)
							settingsindex++;
					}
					else if (encoder->decreased())
					{
						if (settingsindex != 0)
							settingsindex--;
					}
				}
				else if (menuActive) {
					if (encoder->increased())
					{
						if (menuindex != 0)
							menuindex++;
					}
					else if (encoder->decreased())
					{
						if (menuindex != 0)
							menuindex--;
					}
				}
				else {
					if (encoder->increased())
					{
						if (funcindex != 9)
							funcindex++;
					}
					else if (encoder->decreased())
					{
						if (funcindex != 0)
							funcindex--;
					}
				}
			}
			if (encoder->buttonDoublePressed())
			{
				funcType = settings.functypes[funcindex];
				fulloledupdate();
			}
			if (encoder->buttonPressed()) {
				if (!menuActive) {
					Serial.println("ACTIVATE MENU");
					menuindex = 0;
					menuActive = true;
					menuActivationTime = seconds;
					fulloledupdate();
				}
				else {
					menuActivationTime = seconds;// Refresh of seconds
					if (menuindex == 0) {
						settingsActive = true;
						fulloledupdate();
					}
				}
			}
		}
#endif
#if HID_ENABLED
		if (USBLIB->state != oldUsbState)
		{
			Serial.print(F("USBSTATE: "));
			switch (USBLIB->state)
			{
				VERBOSECASELN(USB_UNCONNECTED)
					VERBOSECASELN(USB_ADDRESSED)
					VERBOSECASELN(USB_ATTACHED)
					VERBOSECASELN(USB_POWERED)
					VERBOSECASELN(USB_SUSPENDED)
					VERBOSECASELN(USB_CONFIGURED)
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
	}
}
