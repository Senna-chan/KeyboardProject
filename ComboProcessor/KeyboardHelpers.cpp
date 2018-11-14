// 
// 
// 

#include "KeyboardHelpers.h"



char convertHIDToASCII(byte keyCode)
{
	if (keyCode <= KEY_Z)
	{
		return keyCode + 93; // To
	}
	if (keyCode >= KEY_1 && keyCode <= KEY_9)
	{
		return keyCode + 19;
	}
	if (keyCode == KEY_0)
	{
		return keyCode + 9;
	}
	if (keyCode >= KEYPAD_1 && keyCode <= KEYPAD_9)
	{
		return keyCode - 40;
	}
	if (keyCode == KEYPAD_0)
	{
		return keyCode - 50;
	}
	if (keyCode == KEY_DELETE)
	{
		return KEY_DELETE;
	}
	if (keyCode == KEY_ENTER || keyCode == KEYPAD_ENTER)
	{
		return KEY_ENTER;
	}

#if DEBUG
	Serial.print("Not a char: ");
	Serial.println(KeyboardKeycode_string_table[keyCode]);
	return 0;
#endif
}

/**
 * \brief Waits untill all keys are empty
 */
void waitNoKeysPressed()
{
	while (true)
	{
		bool keysPressed = false;
		matrix.getKeys();
		for (int i = 0; i < LIST_MAX; i++)   // Scan the whole key list.
		{
			if (matrix.key[i].kcode != 0x00)
			{
				keysPressed = true;
			}
		}
		if (!keysPressed) break;
		delay(10);
	}
}

keyReport getKeysFromKeyboard()
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
					Serial.println(F("\tpressed in keys"));
#endif
					if (k >= 224) {
						curKeyReport.modifier |= (1 << (k - 224));
						k = 0;
					}
					if (curKeyReport.keys[0] != k && curKeyReport.keys[1] != k &&
						curKeyReport.keys[2] != k && curKeyReport.keys[3] != k &&
						curKeyReport.keys[4] != k && curKeyReport.keys[5] != k) {

						for (i = 0; i < 6; i++) {
							if (curKeyReport.keys[i] == 0x00) {
								curKeyReport.keys[i] = k;
								break;
							}
						}
					}
					break;
				case RELEASED:
#if DEBUG
					Serial.print(F("Key: "));
					Serial.print((uint8_t)matrix.key[i].kcode);
					Serial.println(F("\treleased"));
#endif
					if (k >= 224) {
						curKeyReport.modifier &= ~(1 << (k - 224));
						k = 0;
					}
					for (i = 0; i < 6; i++) {
						if (0 != k && curKeyReport.keys[i] == k) {
							curKeyReport.keys[i] = 0x00;
							break;
						}
					}
					break;
				case HOLD:
				case IDLE:
					break;
				}
			}
		}
	}
	return curKeyReport;
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
					if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT)
					{
						pressedShift = true;
					}
					else if (k == KEY_CAPS_LOCK)
					{
						pressedShift = !pressedShift;
					}
					else
					{
						c = convertHIDToASCII(k);
					}
					break;
				case RELEASED:
					if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT)
					{
#if DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\treleased in menu"));
#endif
						pressedShift = false;
					}
				case HOLD:
					if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT)
					{
						pressedShift = true;
					}
					break;
				case IDLE:
					if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT)
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