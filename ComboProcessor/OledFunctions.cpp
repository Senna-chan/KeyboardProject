// 
// 
// 

#include <Arduino.h>
#include "OledFunctions.h"
#include "Variables.h"
#include "Helpers.h"
#include "CustomIcons.h"
#include "Adafruit_SSD1306_EXT.h"
#include <Adafruit_SSD1306.h>

void fullupdateoled()
{
	main_oled.clearDisplay();
	redrawhud();
	drawmenu();
	drawfunctype();
	main_oled.display();
}

void redrawhud()
{              
	int w, h;


	main_oled.fillRect(0, 0, 128, 20, BLACK);
	if (operateMode == BLUETOOTH) {
		if(bluetoothConnected)
		{
			main_oled.drawBitmap(0, 0, bluetooth_connected, 16, 16, WHITE);
		}
		else {
			main_oled.drawBitmap(0, 0, bluetooth, 16, 16, WHITE);
		}
	}
	else if (operateMode == CABLE) {
		main_oled.drawBitmap(0, 0, usb, 16, 16, WHITE);
	}
	else if (operateMode == CHARGING) {
		main_oled.drawBitmap(0, 0, charging, 16, 16, WHITE);
	}
	if (batteryvoltage > 4) {
		main_oled.drawBitmap(108, 0, battery_full, 16, 16, WHITE);
	}
	else if (between(batteryvoltage, 3.75, 4)) {
		main_oled.drawBitmap(108, 0, battery_almost_full, 16, 16, WHITE);
	}
	else if (between(batteryvoltage, 3.3, 3.75)) {
		main_oled.drawBitmap(108, 0, battery_half, 16, 16, WHITE);
	}
	else if (between(batteryvoltage, 3, 3.3)) {
		main_oled.drawBitmap(108, 0, battery_almost_empty, 16, 16, WHITE);
	}
	else if (batteryvoltage < 3) {
		main_oled.drawBitmap(108, 0, battery_empty, 16, 16, WHITE);
	}
	main_oled.setCursor(88, 4);
	main_oled.println(batteryvoltage, WHITE);
}

void drawmenu()
{
	if(menuindex == 0)
	{
		drawfunctype();
	} 
	else if(menuindex == 1) // Goto settings?
	{
		
	}
}

void drawfunctype()
{
	if (!sdInitialized) return;
	clearNonHudArea();
	if(menuindex == 0)
	{
		if (oldFuncType == funcType) return;
		if (funcType == FNKEYS) {
			main_oled.setTextSize(5);
			main_oled.setCursor(32, 16);
			main_oled.print("FN");
			main_oled.setTextSize(1);
			return;
		}
		oldFuncType = funcType;
		char *fname = NULL;
		int w, h;
		if (funcType == MEDIA)
		{
			fname = String("modes/media/logo.bmp").begin();
		}
		else if(funcType == PROGRAMMING)
		{
			fname = String("modes/prog/logo.bmp").begin();
		}
		if(fname == NULL)
		{
			Serial.println("Not implemented yet");
		}
		Serial.println(fname);
		if(!SD.exists(fname))
		{
			Serial.println("No logo found");
			return;
		}

		unsigned char* img = read1bitBMP(fname, &w, &h);
		main_oled.drawBitmap(48, 24, img, w, h, WHITE);
	}
}

void clearNonHudArea()
{
	main_oled.fillRect(32, 32, 128, 48, BLACK);
	main_oled.display();
}

bool yesnoScreen(bool defaultval, char* line1, char* line2 = "")
{
	bool returnval = defaultval;
	int yesnoYPos = 44;
	clearNonHudArea();
	if (defaultval)
	{
		main_oled.drawBitmap(32, yesnoYPos, yes_inv, yes_width, yes_height, WHITE);
		main_oled.drawBitmap(72, yesnoYPos, no, yes_width, yes_height, WHITE);
	}
	else
	{
		main_oled.drawBitmap(32, yesnoYPos, yes, yes_width, yes_height, WHITE);
		main_oled.drawBitmap(72, yesnoYPos, no_inv, yes_width, yes_height, WHITE);
	}
	main_oled.drawStringCenter(0, 20, line1, WHITE);
	main_oled.drawStringCenter(0, 30, line2, WHITE);
	main_oled.display();
	while (true) {
		if (currentEncPos != lastEncPos)
		{
			if (currentEncPos > lastEncPos && returnval)
			{
				returnval = false;
				main_oled.drawBitmap(32, yesnoYPos, yes, yes_width, yes_height, WHITE);
				main_oled.drawBitmap(72, yesnoYPos, no_inv, yes_width, yes_height, WHITE);
			}
			else if (currentEncPos < lastEncPos && !returnval)
			{
				returnval = true;
				main_oled.drawBitmap(32, yesnoYPos, yes_inv, yes_width, yes_height, WHITE);
				main_oled.drawBitmap(72, yesnoYPos, no, yes_width, yes_height, WHITE);
			}
			lastEncPos = currentEncPos;
			main_oled.display();
		}
		if (encoder->getButton() == encoder->Clicked) break;
	}
	return returnval;
}

String inputScreen(uint8_t maxLength, char* line)
{
	return inputScreen(maxLength, line, 10, 30, "");
}
String inputScreen(uint8_t maxLength, char* line, String regex)
{
	return inputScreen(maxLength, line, 10, 30, regex);
}

String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty)
{
	return inputScreen(maxLength, line, startx, starty, "");
}

String inputScreen(uint8_t maxLength, char* line, uint16_t startx, uint16_t starty, String regex)
{
	pressedShift = false;
	while (true) {
		clearNonHudArea();
		main_oled.drawStringCenter(64, 20, line, WHITE);

		String value = "";
		char c = 0;
		main_oled.setCursor(startx, starty);
		main_oled.display();
		while (true) {
			if (encoder->getButton() == encoder->DoubleClicked || value.length() == maxLength) break;
			c = getAsciiFromKeyboard();
			if (c != 0) {
				Serial.print(c); Serial.print(" ");
				if (c == HID_KEYBOARD_DELETE) // backspace
				{
					value.substring(0, value.length() - 1); // Removes last character
					main_oled.setCursor(main_oled.getCursorX() - 6, main_oled.getCursorY()); // Resets cursor to prev position
					main_oled.fillRect(main_oled.getCursorX(), main_oled.getCursorY(), 6, 7, BLACK); // Clears char
					main_oled.drawFastHLine(0, main_oled.getCursorY() + 8, 128, BLACK); // Clear underline
					main_oled.drawFastHLine(main_oled.getCursorX(), main_oled.getCursorY() + 8, 6, WHITE);
					main_oled.display();
					continue;
				}
				else if (c == KEY_ENTER)
				{
					break;
				}
				if (regex.indexOf(c) == -1) continue; // Illegal char
				value += c;
				main_oled.print(c);
				main_oled.drawFastHLine(0, main_oled.getCursorY() + 8, 128, BLACK); // Clear underline
				main_oled.drawFastHLine(main_oled.getCursorX(), main_oled.getCursorY() + 8, 6, WHITE);
				main_oled.display();

				Serial.print(pressedShift);
				Serial.print(" value:"); Serial.print(value);
				Serial.print(" length: "); Serial.println(value.length());
			}
		}
		if (yesnoScreen(true, "Is this input good?", value.begin())) return value;
	}
}

char convertHIDToASCII(byte keyCode)
{
	if(keyCode <= KEY_Z)
	{
		return keyCode + 93; // To
	} else if(keyCode >= KEY_1 && keyCode <= KEY_9)
	{
		return keyCode + 19;
	} else if(keyCode == KEY_0)
	{
		return keyCode + 9;
	}
	
	else if(keyCode >= KEYPAD_1 && keyCode <= KEYPAD_9)
	{
		return keyCode - 40;
	} else if(keyCode == KEYPAD_0)
	{
		return keyCode - 50;
	}
	
	else if(keyCode == HID_KEYBOARD_DELETE)
	{
		return HID_KEYBOARD_DELETE;
	} else if(keyCode == HID_KEYBOARD_ENTER || keyCode == HID_KEYPAD_ENTER)
	{
		return HID_KEYBOARD_ENTER;
	}
	Serial.print("Not a char: ");
	Serial.println(keyCode);
	return 0;
}

char getKeysFromKeyboard(uint8_t* modifiers)
{
	char c = 0;
	if (matrix.getKeys())
	{
		for (int i = 0; i < LIST_MAX; i++)   // Scan the whole key list.
		{
			if (matrix.key[i].stateChanged)   // Only find keys that have changed state.
			{
				byte k = matrix.key[i].kcode;
				switch (matrix.key[i].kstate) {
					case PRESSED:
						#if DEBUG
							Serial.print(F("Key: "));
							Serial.print((uint8_t)matrix.key[i].kcode);
							Serial.println(F("\tpressed in menu"));
						#endif
							if (k >= 224)
							{
								*modifiers |= 1 << (k - 224);
							} else
							{
								c = convertHIDToASCII(k);
							}
						break;
					case RELEASED:
						#if DEBUG
							Serial.print(F("Key: "));
							Serial.print((uint8_t)matrix.key[i].kcode);
							Serial.println(F("\treleased"));
						#endif
						if (k >= 224)
						{
							*modifiers &= ~(1 << (k - 224));
						}
						break;
					case HOLD:
					case IDLE:
						break;
				}
			}
		}
	}
	return c;
}


char getAsciiFromKeyboard() {
	char c = 0;
	if (matrix.getKeys())
	{
		for (int i = 0; i < 2; i++)   // Scan only 2 keys
		{
			if (matrix.key[i].stateChanged || matrix.key[i].kstate == HOLD)   // Only find keys that have changed state or that are holded
			{
				byte k = matrix.key[i].kcode;
				switch (matrix.key[i].kstate) {
					case PRESSED:
						#if DEBUG
							Serial.print(F("Key: "));
							Serial.print((uint8_t)matrix.key[i].kcode);
							Serial.println(F("\tpressed in menu"));
						#endif
							if (k == HID_KEYBOARD_LEFT_SHIFT || k == HID_KEYBOARD_RIGHT_SHIFT)
							{
								pressedShift = true;
							} else if(k == HID_KEYBOARD_CAPS_LOCK)
							{
								pressedShift = !pressedShift;
							}
							else
							{
								c = convertHIDToASCII(k);
							}
						break;
					case RELEASED:
						if (k == HID_KEYBOARD_LEFT_SHIFT || k == HID_KEYBOARD_RIGHT_SHIFT)
						{
#if DEBUG
							Serial.print(F("Key: "));
							Serial.print((uint8_t)matrix.key[i].kcode);
							Serial.println(F("\treleased in menu"));
#endif
							pressedShift = false;
						}
					case HOLD:
						if (k == HID_KEYBOARD_LEFT_SHIFT || k == HID_KEYBOARD_RIGHT_SHIFT)
						{
							pressedShift = true;
						}
						break;
					case IDLE:
						if (k == HID_KEYBOARD_LEFT_SHIFT || k == HID_KEYBOARD_RIGHT_SHIFT)
						{
							pressedShift = false;
						}
						break;
				}
			}
		}
	}
	if (pressedShift && (c >= 'a' && c <= 'z')) c -= 0x20; // From lowercase to uppercase
	return c;
}