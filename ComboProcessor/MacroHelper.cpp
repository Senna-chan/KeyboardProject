// 
// 
// 

#include "MacroHelper.h"
#include "Variables.h"
#include "Helpers.h"
#include "HIDFunctions.h"
#include "KeyboardHelpers.h"

void loadMacros()
{
	if (sdInitialized)
	{
		String path = "modes/"+funcType+"/";
		if (!SD.exists((path + "macros")))
		{
			Serial.println(F("No macro's found"));
			return;
		}
		FsFile f = SD.open((path + "macros.txt"));
		if (f.size() != 9 * 7 * macroKeyDepth)
		{
			Serial.println(F("Not a valid macro file"));
			f.close();
			return;
		}
		for (int i = 0; i < 9; i++) // Macro
		{
			for (int j = 0; j < macroKeyDepth; j++) // Macro Keys
			{
				keyMacros[i][j].modifier = f.read();
				for (int k = 0; k < 6; k++) // Key
				{
					keyMacros[i][k].keys[k] = f.read();
				}
			}
		}
		f.close();
	}
}

void saveMacro()
{
	if (sdInitialized) {
		String path = "modes/" + funcType + "/";
		if (SD.exists(path + "macros")) SD.remove(path + "macros");
		FsFile f = SD.open((path + "macros.txt"), O_RDWR);
		int16_t result = f.write(keyMacros, sizeof(KeyReport) * 9 * 9);
		if(result != -1)
		{
#if DEBUG
			Serial.print(F("Success. Wrote "));
			Serial.print(result);
			Serial.println(F(" bytes"));
#endif
		}
		f.close();
	}
}


void genMacro(uint8_t macroPos)
{
	for (int i = 0; i < macroKeyDepth; i++) {
		keyMacros[macroPos][i] = getMacroKeysFromKeyboard();
		if (encoder->buttonDoublePressed()) break;
	}
	Serial.print(F("BREAK"));
	saveMacro();
}

void pressMacro(uint8_t macroPos)
{
	for(auto& keys : keyMacros[macroPos])
	{
		if (keys.keys[0] != 0x00) {
			writeKeyReport(keys);
			delay(50);
		}
	}
}


KeyReport getMacroKeysFromKeyboard()
{
	KeyReport macroReport;
	uint16_t oriMatrixHoldTime = matrix.getHoldTime(); // Original hold time 
	matrix.setHoldTime(2000); // 2 seconds
	releaseAllKeyboardMouse();
	waitNoKeysPressed();
	Serial.println(F("Press the keys"));
	while (true) {
		handleEncoder();
		if (encoder->buttonPressed() || encoder->buttonDoublePressed()) break;
		bool holded = true;
		if (matrix.getKeys())
		{
			for (int i = 0; i < LIST_MAX; i++)   // Scan the whole key list.
			{
				if (matrix.key[i].stateChanged)   // Only find keys that have changed state.
				{
					byte k = matrix.key[i].kcode;
					switch (matrix.key[i].kstate) {
					case PRESSED:
						holded = false;
#if DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\tpressed for macro"));
#endif

						if (k >= 224) {
							macroReport.modifier |= (1 << (k - 224));
							k = 0;
						}
						if (macroReport.keys[0] != k && macroReport.keys[1] != k &&
							macroReport.keys[2] != k && macroReport.keys[3] != k &&
							macroReport.keys[4] != k && macroReport.keys[5] != k) {

							for (i = 0; i < 6; i++) {
								if (macroReport.keys[i] == 0x00) {
									macroReport.keys[i] = k;
									break;
								}
							}
						}
						break;
					case RELEASED:
						holded = false;
#if DEBUG
						Serial.print(F("Key: "));
						Serial.print((uint8_t)matrix.key[i].kcode);
						Serial.println(F("\treleased"));
#endif

						if (k >= 224) {
							macroReport.modifier &= ~(1 << (k - 224));
							k = 0;
						}
						for (i = 0; i < 6; i++) {
							if (0 != k && macroReport.keys[i] == k) {
								macroReport.keys[i] = 0x00;
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
		else
		{
			holded = false;
			continue;
		}
		Serial.print(F("keyReport"));
		Serial.print(F("\tmodifier\t")); Serial.print(macroReport.modifier);
		Serial.print(F("\tkeys\t"));
		for (auto& key : macroReport.keys) {
			Serial.print(key); Serial.print(F(" "));
		}
		Serial.println();
		if (holded) break;
	}
	matrix.setHoldTime(oriMatrixHoldTime); // Revert back to original hold time
	Serial.println(F("Now release the keys"));
	waitNoKeysPressed();
	return macroReport;
}





void renderMacroKeysOled(KeyReport keyreport)
{
	
}
