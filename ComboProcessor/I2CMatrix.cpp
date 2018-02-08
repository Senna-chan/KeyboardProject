// 
// 
// 

#include "I2CMatrix.h"

I2CMatrixClass::I2CMatrixClass(){}

I2CMatrixClass::I2CMatrixClass(KeyboardKeycode *userKeymap, Adafruit_MCP23017 rowchip, Adafruit_MCP23017 colchip, byte numRows, byte numCols) {
	rowChip = rowchip;
	colChip = colchip;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;
	keymap = userKeymap;
	setDebounceTime(10);
	setHoldTime(500);
	keypadEventListener = 0;

	startTime = 0;
	single_key = false;
}

// Initialize the chips
void I2CMatrixClass::begin() {
	
	for (int i = 0; i < 16; i++) {
		if (i < 14) {
			rowChip.pinMode(i, INPUT);
			rowChip.pullUp(i, HIGH);
		}
		colChip.pinMode(i, OUTPUT);
		colChip.pullUp(i, HIGH);
		colChip.digitalWrite(i, HIGH);
	}
}

// Returns a single key only. Retained for backwards compatibility.
char I2CMatrixClass::getKey() {
	single_key = true;

	if (getKeys() && key[0].stateChanged && (key[0].kstate == PRESSED))
		return key[0].kcode;

	single_key = false;

	return KEY_RESERVED;
}

// Populate the key list.
bool I2CMatrixClass::getKeys() {
	bool keyActivity = false;

	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ((millis() - startTime)>debounceTime) {
		scanKeys();
		keyActivity = updateList();
		startTime = millis();
	}

	return keyActivity;
}

// Private : Hardware scan
void I2CMatrixClass::scanKeys() {
	// bitMap stores ALL the keys that are being pressed.
	//rowChip.writeRegister(0xFF, MCP23017_IODIRA);
	//rowChip.writeRegister(0xFF, MCP23017_GPPUA);
	//rowChip.writeRegister(0xFF, MCP23017_IODIRB);
	//rowChip.writeRegister(0xFF, MCP23017_GPPUB);

	// bitMap stores ALL the keys that are being pressed.
	for (byte c = 0; c<sizeKpd.columns; c++) {
		colChip.pinMode(c, OUTPUT);
		colChip.digitalWrite(c, LOW);	// Begin column pulse output.
		byte rowPinValues = rowChip.readGPIO(0);
//		if (rowPinValues != B11111111) {
//			Serial.print("c:");
//			Serial.print(c);
//			Serial.print("\trv:");
//			for (unsigned int mask = 0x80; mask; mask >>= 1) {
//				Serial.print(mask&rowPinValues ? '1' : '0');
//			}
//			Serial.println();
//		}
		for (byte r = 0; r<sizeKpd.rows; r++) {
			bitWrite(bitMap[r], c, !getBit(rowPinValues, r));  // keypress is active low so invert to high.
		}
//		for (byte r = 0; r<sizeKpd.rows; r++) {
//			bitWrite(bitMap[r], c, pin_read(r+8, false));  // keypress is active low so invert to high.
//		}
		// Set pin to high impedance input. Effectively ends column pulse.
		colChip.digitalWrite(c, HIGH);	// Begin column pulse output.
		colChip.pinMode(c, INPUT);
	}
}

// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool I2CMatrixClass::updateList() {

	bool anyActivity = false;

	// Delete any IDLE keys
	for (byte i = 0; i<LIST_MAX; i++) {
		if (key[i].kstate == IDLE) {
			key[i].kcode = KEY_RESERVED;
			key[i].kpos = -1;
			key[i].stateChanged = false;
		}
	}

	// Add new keys to empty slots in the key list.
	for (byte r = 0; r<sizeKpd.rows; r++) {
		for (byte c = 0; c < sizeKpd.columns; c++) {
			bool button = bitRead(bitMap[r], c);
			KeyboardKeycode keyCode = keymap[r * sizeKpd.columns + c];
			if (keyCode == KEY_RESERVED) continue; // Ignore 0 keys
			int keyPos = r * sizeKpd.columns + c;
			int idx = findInList(keyPos);
			// MyKey is already on the list so set its next state.
			if (idx > -1) {
				nextKeyState(idx, button);
			}
			// MyKey is NOT on the list so add it.
			if ((idx == -1) && button) {
				for (byte i = 0; i<LIST_MAX; i++) {
					if (key[i].kcode == KEY_RESERVED) {		// Find an empty slot or don't add key to list.
						key[i].kcode = keyCode;
						key[i].kpos = keyPos;
						key[i].kstate = IDLE;		// Keys NOT on the list have an initial state of IDLE.
						nextKeyState(i, button);
						break;	// Don't fill all the empty slots with the same key.
					}
				}
			}
		}
	}

	// Report if the user changed the state of any key.
	for (byte i = 0; i<LIST_MAX; i++) {
		if (key[i].stateChanged) anyActivity = true;
	}

	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
void I2CMatrixClass::nextKeyState(byte idx, boolean button) {
	key[idx].stateChanged = false;

	switch (key[idx].kstate) {
	case IDLE:
		if (button == CLOSED) {
			transitionTo(idx, PRESSED);
			holdTimer = millis();
		}		// Get ready for next HOLD state.
		break;
	case PRESSED:
		if ((millis() - holdTimer)>holdTime)	// Waiting for a key HOLD...
			transitionTo(idx, HOLD);
		else if (button == OPEN)				// or for a key to be RELEASED.
			transitionTo(idx, RELEASED);
		break;
	case HOLD:
		if (button == OPEN)
			transitionTo(idx, RELEASED);
		break;
	case RELEASED:
		transitionTo(idx, IDLE);
		break;
	}
}

// New in 2.1
bool I2CMatrixClass::isPressed(char keyChar) {
	for (byte i = 0; i<LIST_MAX; i++) {
		if (key[i].kcode == keyChar) {
			if ((key[i].kstate == PRESSED) && key[i].stateChanged)
				return true;
		}
	}
	return false;	// Not pressed.
}

// Search by character for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int I2CMatrixClass::findInList(char keyChar) {
	for (byte i = 0; i<LIST_MAX; i++) {
		if (key[i].kcode == keyChar) {
			return i;
		}
	}
	return -1;
}

// Search by code for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int I2CMatrixClass::findInList(int keyCode) {
	for (byte i = 0; i<LIST_MAX; i++) {
		if (key[i].kpos == keyCode) {
			return i;
		}
	}
	return -1;
}

// New in 2.0
char I2CMatrixClass::waitForKey() {
	char waitKey = KEY_RESERVED;
	while ((waitKey = getKey()) == KEY_RESERVED);	// Block everything while waiting for a keypress.
	return waitKey;
}

// Backwards compatibility function.
KeyState I2CMatrixClass::getState() {
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool I2CMatrixClass::keyStateChanged() {
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a Key object.
byte I2CMatrixClass::numKeys() {
	return sizeof(key) / sizeof(Key);
}

// Minimum debounceTime is 1 mS. Any lower *will* slow down the loop().
void I2CMatrixClass::setDebounceTime(uint debounce) {
	debounce<1 ? debounceTime = 1 : debounceTime = debounce;
}

void I2CMatrixClass::setHoldTime(uint hold) {
	holdTime = hold;
}

void I2CMatrixClass::addEventListener(void(*listener)(char)) {
	keypadEventListener = listener;
}

void I2CMatrixClass::transitionTo(byte idx, KeyState nextState) {
	key[idx].kstate = nextState;
	key[idx].stateChanged = true;

	// Sketch used the getKey() function.
	// Calls keypadEventListener only when the first key in slot 0 changes state.
	if (single_key) {
		if ((keypadEventListener != NULL) && (idx == 0)) {
			keypadEventListener(key[0].kcode);
		}
	}
	// Sketch used the getKeys() function.
	// Calls keypadEventListener on any key that changes state.
	else {
		if (keypadEventListener != NULL) {
			keypadEventListener(key[idx].kcode);
		}
	}
}

I2CMatrixClass I2CMatrix;
