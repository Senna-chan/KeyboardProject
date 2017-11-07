#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DuckyParser.h>
#include <Adafruit_MCP23017.h>
#include <Wire.h>
#include "I2CMatrix.h"
#include "Key.h"
#include <TimedAction.h>
#include "PS2Mouse.h"
#include <ClickEncoder.h>
#include "KeyboardCodes.h"
#include "ConsumerKeyCodes.h"
#include "SerialAnything.h"

#define DATA_PIN 5 // green
#define CLOCK_PIN 6 // white
#define SD_ENABLE 10
#define OPERATEMODEPIN PB1
#define analogReadVoltage(pin) analogRead(pin) * (5.0 / 1023)

const int numOfScreens = 10;

HardwareTimer timer(1);
#define OLED_DC     6
#define OLED_CS     7
#define OLED_RESET  8
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
HardwareSerial HIDSerial = Serial2;
ClickEncoder *encoder;
int16_t lastEncPos, currentEncPos;
void timerIsr() {
	encoder->service();
}

int currentScreen = 0;
String screens[numOfScreens][2] = { { "Function Mode" },{ "Active Mode" },{ "Active BTAddr" },
{ "Set BTAddr" },{ "Overload Temp.","degC" },{ "Accel Time", "Secs" },{ "Restart Time","Mins" },
{ "Analog Out. Curr.","mA" },{ "Input Temp.","degC" },{ "Run Time", "Hours" } };
int parameters[numOfScreens];
#define resetToHomeScreen 5000
bool checkResetHomeScreen, doValueChange;
//LiquidCrystal_PCF8574 lcd(0x27);
enum FunctionType { MEDIA, ONEPRESS };
enum OperateMode { USB, BLUETOOTH, CHARGING };
enum KeyboardLeds : uint8_t {
	LED_NUM_LOCK = (1 << 0),
	LED_CAPS_LOCK = (1 << 1),
	LED_SCROLL_LOCK = (1 << 2)
};
byte leds;
Adafruit_MCP23017 rowChip, colChip;
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

ConsumerKeycode mediakeys[9]{ HID_CONSUMER_VOLUME_DECREMENT, HID_CONSUMER_MUTE, HID_CONSUMER_VOLUME_INCREMENT, MEDIA_PREVIOUS, MEDIA_PLAY_PAUSE, MEDIA_NEXT, /*Opens media player*/ HID_CONSUMER_AL_AUDIO_PLAYER, };
//PS2Mouse ps2_mouse(CLOCK_PIN, DATA_PIN);
bool leftMouseButton = 0;
bool rightMouseButton = 0;
TimedAction doMouseStuffAction = TimedAction();
I2CMatrixClass matrix;
float batteryVoltage;
FunctionType funcType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
FunctionType oldFuncType = MEDIA; // 0 = function buttons are media keys and no special keys 1 = Function buttons are duckyscripts and certain keys are special keys
OperateMode operateMode;
uint16_t spiBuffer[52];
struct Settings {
	byte btaddr1[6];
	byte btaddr2[6];
	byte btaddr3[6];
	uint16_t mouseaccel[6];
};

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
	Mouse.move(data.position.x, data.position.y);*/
}

void printScreen() {
	/*lcd.clear();
	lcd.print(screens[currentScreen][0]);
	lcd.setCursor(0, 1);
	switch (currentScreen) {
	case 0:
	switch (funcType) {
	case FunctionType::MEDIA:
	lcd.print("Media");
	break;
	case FunctionType::ONEPRESS:
	lcd.print("FNKeys/Ducky");
	break;
	}
	break;
	default:
	lcd.print(parameters[currentScreen]);
	lcd.print(" ");
	lcd.print(screens[currentScreen][1]);
	lcd.setCursor(0, 1);
	break;
	}*/
}
void checkForLeds() {
	if (leds & LED_CAPS_LOCK)
		rowChip.digitalWrite(15, LOW);
	else
		rowChip.digitalWrite(15, HIGH);
	if (leds & LED_NUM_LOCK)
		rowChip.digitalWrite(14, LOW);
	else
		rowChip.digitalWrite(14, HIGH);
	if (leds & LED_SCROLL_LOCK)
		rowChip.digitalWrite(13, LOW);
	else
		rowChip.digitalWrite(13, HIGH);
}

void setup() {
	Serial.begin(115200);
	while (!Serial); // We want Serial to debug
	HIDSerial.begin(1000000);
	Serial.println(F("Initing Wire things"));
	Wire.setClock(400000);
	Wire.begin();
	rowChip.begin(B000);
	colChip.begin(B100);
	rowChip.writeRegister(MCP23017_IODIRB, 0x00); // Set pin 7-15 to output
	rowChip.digitalWrite(13, HIGH);
	rowChip.digitalWrite(14, HIGH);
	rowChip.digitalWrite(15, HIGH);
	Serial.println(F("Wire initialized"));
	//Serial.println(F("Initializing PS2 Mouse"));
	//ps2_mouse.initialize(); // If it is not connected DO NOT CALL ANY PS2 MOUSE FUNCTION
	//doMouseStuffAction = TimedAction(10, *doMouseStuff);
	//Serial.println(F("PS2 Mouse Initialized"));
	Serial.println(F("Initializing HID"));
	Serial.println(F("HID Initilized"));
	Serial.println(F("Initializing DuckyParser"));
	if (duckyParser.init(SD_ENABLE)) {
		Serial.println(F("DuckyParser Initialized"));
	}
	else {
		Serial.println(F("DuckyParser failed to Initialized"));
	}
	//Serial.println(F("Initializing Encoder"));
	//encoder = new ClickEncoder(A2, A3, 8);  
	//encoder->setAccelerationEnabled(false);
	//encoder->setDoubleClickEnabled(true);
	timer.pause();
	timer.setPeriod(1000);
	// Set up an interrupt on channel 1
	timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
	timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
	timer.attachCompare1Interrupt(timerIsr);
	timer.refresh(); // Refresh the timer's count, prescale, and overflow
	timer.resume(); // Start the timer counting
	//lastEncPos = -1;
	//Serial.println(F("Encoder Initialized"));
	//Serial.println(F("Initializing LCD"));
	//lcd.begin(16, 2);
	//lcd.setBacklight(255);
	//lcd.clear();
	//printScreen();
	//Serial.println(F("LCD Initialized"));
	Serial.println(F("Initializing Matrix"));
	matrix = I2CMatrixClass(makeKeymap(keys), rowChip, colChip, ROWS, COLS);
	matrix.setDebounceTime(1);
	Serial.println(F("Matrix Initialized"));
	Serial.println(F("Initializing Done, Type away"));
}

void handleEncoder() {
	currentEncPos += encoder->getValue();

	if (currentEncPos != lastEncPos && currentEncPos % 2) {
		if (currentEncPos < lastEncPos) {
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
		printScreen();
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
void checkKeys() {
	if (matrix.getKeys())
	{
		for (int i = 0; i<LIST_MAX; i++)   // Scan the whole key list.
		{
			HIDSerial.write('k');
			if (matrix.key[i].stateChanged)   // Only find keys that have changed state.
			{
				HIDSerial.write('k');
				HIDSerial.write(matrix.key[i].kcode);
				switch (matrix.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
				case PRESSED:
					/*Serial.print("Key: ");
					Serial.print((uint8_t)matrix.key[i].kcode);
					Serial.print("\t\ton pos: ");
					Serial.print(matrix.key[i].kpos);
					Serial.println("\tpressed");*/
					HIDSerial.write(matrix.key[i].kstate);
					break;
				case RELEASED:
					/*Serial.print("Key: ");
					Serial.print((uint8_t)matrix.key[i].kcode);
					Serial.print("\t\ton pos: ");
					Serial.print(matrix.key[i].kpos);
					Serial.println("\treleased");*/
					HIDSerial.write(matrix.key[i].kstate);
					break;
				case IDLE:
				case HOLD:
					break;
				}
				HIDSerial.write(i);
			}
		}
	}
}
void checkHIDSerial() {
	while (HIDSerial.available()) {
		char command = HIDSerial.read();
		switch (command) {
		case 'l'://Leds
			leds = Serial.read();
			break;
		case'o': // OperateMode
			operateMode = (OperateMode)Serial.read();
			break;
		case 'b': // Battery status
			uint16_t tempBat;
			Serial2_readAnything(tempBat);
			batteryVoltage = tempBat / 100;
			break;
		default:
			break;
		}
	}
}
void loop() {
	checkHIDSerial();
	//doMouseStuffAction.check();
	checkForLeds();
	checkKeys();
	//handleEncoder();
}
