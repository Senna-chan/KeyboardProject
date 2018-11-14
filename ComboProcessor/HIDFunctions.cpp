// 
// 
// 

#include "HIDFunctions.h"
#include "Variables.h"
#include <USBHID.h>
#include "Config.h"
#include <Arduino.h>

/**
 * \brief Release EVERYTHING and resets mouse
 */
void releaseAllKeyboardMouse()
{
	releaseAllKeyboardKeys();
	moveMouse(0, 0, 0, 0);
	releaseMouseAll();
}


void pressMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if (!bt->mouseIsPressed(button)) bt->mousePress(button);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		if (!Mouse.isPressed(button)) Mouse.press(button);
	}
}

void releaseMouse(uint8_t button) {
	if (operateMode == BLUETOOTH) {
		if (!bt->mouseIsPressed(button))  bt->mouseRelease(button);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		if (Mouse.isPressed(button)) Mouse.release(button);
	}
}

void releaseMouseAll()
{
	if (operateMode == BLUETOOTH) {
		bt->mouseReleaseAll();
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Mouse.release(0xFF);
	}
}

void moveMouse(byte x, byte y, byte wheelx, byte wheely)
{
	moveMouse(x, y);
	moveScrollWheel(wheelx, wheely);
}


void moveMouse(byte x, byte y) {
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
	if (operateMode == BLUETOOTH) {
		bt->keyboardPress(key);
	}
	else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
		Keyboard.press(key);
	}
}
void releaseKeyboardKey(uint8_t key) {
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

void writeKeyReport(keyReport report)
{
	if(report.modifier == 0xFF) // Consumer key
	{
		writeConsumerKey(makeWord(report.keys[0], report.keys[1]));
	}
	else {
		if (operateMode == BLUETOOTH) {
			bt->keyReport.modifiers = report.modifier;
			bt->keyReport.keys[0] = report.keys[0];
			bt->keyReport.keys[1] = report.keys[1];
			bt->keyReport.keys[2] = report.keys[2];
			bt->keyReport.keys[3] = report.keys[3];
			bt->keyReport.keys[4] = report.keys[4];
			bt->keyReport.keys[5] = report.keys[5];
			bt->writeKeyReport();
			bt->keyboardReleaseAll();
		}
		else if (operateMode == CABLE && USBLIB->state == USB_CONFIGURED) {
			Keyboard.keyReport.modifiers = report.modifier;
			Keyboard.keyReport.keys[0] = report.keys[0];
			Keyboard.keyReport.keys[1] = report.keys[1];
			Keyboard.keyReport.keys[2] = report.keys[2];
			Keyboard.keyReport.keys[3] = report.keys[3];
			Keyboard.keyReport.keys[4] = report.keys[4];
			Keyboard.keyReport.keys[5] = report.keys[5];
			Keyboard.sendReport();
			Keyboard.releaseAll();
		}
	}
}
