// 
// 
// 

#include "HIDFunctions.h"
#include "Variables.h"
#include <BPLib.h>
#include <USBHID.h>
#include "Config.h"
#include <Arduino.h>

void PressMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if (!bt->mouseIsPressed(button)) bt->mousePress(button);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		if (!Mouse.isPressed(button)) Mouse.press(button);
	}
}

void ReleaseMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if (!bt->mouseIsPressed(button))  bt->mouseRelease(button);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		if (Mouse.isPressed(button)) Mouse.release(button);
	}
}

void MoveMouse(byte x, byte y) {
	if (operateMode == BLUETOOTH) {
		bt->mouseMove(x, y);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Mouse.move(x, y);
	}
}

void moveScrollWheel(byte x, byte y)
{
	if (operateMode == BLUETOOTH) {
		bt->mouseWheel(y);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Mouse.move(0, 0,y, x);
	}
}

void printKeyboardString(String string)
{
	if (operateMode == BLUETOOTH) {
		bt->sendString(string);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.print(string);
	}
}

void pressKeyboardKey(uint8_t key) {
	if (key == 0x00) return;
#if DEBUGKEYBOARD
	Serial.print("pressing 0x");
	Serial.println(key, HEX);
#endif
	if (operateMode == BLUETOOTH) {
		bt->keyboardPress(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.press(key);
	}
}
void releaseKeyboardKey(uint8_t key) {
#if DEBUGKEYBOARD
	Serial.print("releasing 0x");
	Serial.println(key, HEX);
#endif
	if (key == 0x00) return;
	if (operateMode == BLUETOOTH) {
		bt->keyboardRelease(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.release(key);
	}
}

void releaseAllKeyboardKeys()
{
	if (operateMode == BLUETOOTH) {
		bt->keyboardReleaseAll();
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.releaseAll();
	}
}

void writeKeyboardKey(uint8_t key) {
	if (key == 0x00) return;
	if (operateMode == BLUETOOTH) {
		bt->keyboardWrite(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.write(key);
	}
}
void pressConsumerKey(uint16_t key) {
	if (key == 0x00) return;
	if (operateMode == BLUETOOTH) {
		bt->sendConsumer(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Consumer.press(key);
	}
}
void releaseConsumerKey() {
	if (operateMode == BLUETOOTH) {
		bt->releaseConsumer();
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Consumer.release();
	}
}
void writeConsumerKey(uint16_t key) {
	if (key == 0x00) return;
	if (operateMode == BLUETOOTH) {
		bt->writeConsumer(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Consumer.write(key);
	}
}