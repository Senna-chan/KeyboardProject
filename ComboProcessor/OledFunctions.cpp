// 
// 
// 

#include "OledFunctions.h"
#include <USBHID.h>
#include "Config.h"
#include "Variables.h"
#include "Helpers.h"
#include "CustomIcons.h"
#include "Adafruit_SSD1306_EXT.h"
#include <Adafruit_SSD1306.h>
#include "KeyboardHelpers.h"

void fulloledupdate()
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

	if(sdInitialized)
	{
		main_oled.drawBitmap(60, 2, sd_card, sd_card_width, sd_card_height, WHITE);
	}
	else
	{
		main_oled.drawBitmap(60, 0, no_sd_card, no_sd_card_width, no_sd_card_height, WHITE);
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
	main_oled.setCursor(80, 4);
	main_oled.print(batteryvoltage);
}

void drawmenu()
{
	clearNonHudArea();
	if (menuActive) {
		main_oled.setTextSize(2);
		main_oled.drawStringCenter(0, 20, MenuScreens[menuindex][0], WHITE);
		main_oled.drawStringCenter(0, 30, MenuScreens[menuindex][1], WHITE);
		main_oled.setTextSize(1);
		return;
	}
	else if (settingsActive) {
		main_oled.drawStringCenter(64, 20, SettingsScreens[settingsindex][0], WHITE);
		main_oled.drawStringCenter(64, 30, SettingsScreens[settingsindex][1], WHITE);
	}
	else {
		drawfunctype();
	}
}

void drawfunctype()
{
	if (!sdInitialized) return; // We need SD here
	clearNonHudArea();
	if (oldFuncType == funcType) return;
	if (funcType == "fnkeys") {
		main_oled.setTextSize(5);
		main_oled.setCursor(32, 16);
		main_oled.print("FN");
		main_oled.setTextSize(1);
		return;
	}
	oldFuncType = funcType;
	char *fname = NULL;
	int w, h;
	String path = "modes/" + funcType.toLowerCase() + "/";
	Serial.println(fname);
	if(!SD.exists(fname))
	{
		Serial.println("No logo found");
		main_oled.setTextSize(3);
		main_oled.drawStringCenter(48, 24, funcType.c_str(), WHITE);
		main_oled.setTextSize(1);
		return;
	}

	unsigned char* mainLogo = read1bitBMP((path+"logo.bmp").begin(), &w, &h);
	main_oled.drawBitmap(48, 24, mainLogo, w, h, WHITE);

	oled12.drawBitmap(10, 10, read1bitBMP((path + "1.bmp").begin(), &w, &h), w, h, WHITE);
	oled12.drawBitmap(40, 10, read1bitBMP((path + "2.bmp").begin(), &w, &h), w, h, WHITE);
	oled34.drawBitmap(10, 10, read1bitBMP((path + "3.bmp").begin(), &w, &h), w, h, WHITE);
	oled34.drawBitmap(40, 10, read1bitBMP((path + "4.bmp").begin(), &w, &h), w, h, WHITE);
	oled56.drawBitmap(10, 10, read1bitBMP((path + "5.bmp").begin(), &w, &h), w, h, WHITE);
	oled56.drawBitmap(40, 10, read1bitBMP((path + "6.bmp").begin(), &w, &h), w, h, WHITE);
	oled78.drawBitmap(10, 10, read1bitBMP((path + "7.bmp").begin(), &w, &h), w, h, WHITE);
	oled78.drawBitmap(40, 10, read1bitBMP((path + "8.bmp").begin(), &w, &h), w, h, WHITE);
	oled9 .drawBitmap(10, 10, read1bitBMP((path + "9.bmp").begin(), &w, &h), w, h, WHITE);
}



void clearNonHudArea()
{
	main_oled.fillRect(32, 32, 128, 48, BLACK);
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
	main_oled.drawStringCenter(64, 20, line1, WHITE);
	main_oled.drawStringCenter(64, 30, line2, WHITE);
	main_oled.display();
	while (true) {
		handleEncoder();
		if (currentEncPos != lastEncPos)
		{
			if (currentEncPos > lastEncPos && returnval)
			{
				main_oled.fillRect(32, yesnoYPos, yes_width, yes_height, BLACK); // Clear yes no part
				main_oled.fillRect(72, yesnoYPos, yes_width, yes_height, BLACK); // Clear yes no part
				returnval = false;
				main_oled.drawBitmap(32, yesnoYPos, yes, yes_width, yes_height, WHITE);
				main_oled.drawBitmap(72, yesnoYPos, no_inv, yes_width, yes_height, WHITE);
			}
			else if (currentEncPos < lastEncPos && !returnval)
			{
				main_oled.fillRect(32, yesnoYPos, yes_width, yes_height, BLACK); // Clear yes no part
				main_oled.fillRect(72, yesnoYPos, yes_width, yes_height, BLACK); // Clear yes no part
				returnval = true;
				main_oled.drawBitmap(32, yesnoYPos, yes_inv, yes_width, yes_height, WHITE);
				main_oled.drawBitmap(72, yesnoYPos, no, yes_width, yes_height, WHITE);
			}
			main_oled.display();
		}
		if (encoder->buttonPressed()) break;
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
			handleEncoder();
			if (encoder->readStatus(PUSHD) || value.length() == maxLength) break;
			c = getAsciiFromKeyboard();
			if (c != 0) {
				Serial.print(c); Serial.print(" ");
				if (c == KEY_DELETE) // backspace
				{
					value = value.substring(0, value.length() - 1); // Removes last character
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
