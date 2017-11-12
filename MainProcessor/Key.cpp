#include "Key.h"
// default constructor
Key::Key() {
	kcode = KEY_RESERVED;
	kstate = IDLE;
	stateChanged = false;
}

// constructor
Key::Key(KeyboardKeycode userKeyChar) {
	kcode = userKeyChar;
	kpos = -1;
	kstate = IDLE;
	stateChanged = false;
}


void Key::key_update (KeyboardKeycode userKeyChar, KeyState userState, boolean userStatus) {
	kcode = userKeyChar;
	kstate = userState;
	stateChanged = userStatus;
}