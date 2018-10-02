// 
// 
// 

#include "SettingsHelper.h"
#include <ArduinoJson.h>

const char* settingsFile = "settings.json";

void loadSettings() {
	// Open file for reading
	FsFile file = SD.open(settingsFile);

	DynamicJsonBuffer jsonBuffer;

	// Parse the root object
	JsonObject &root = jsonBuffer.parseObject(file);

	if (!root.success()) {
		Serial.println(F("Failed to read file, using default configuration"));
		file.close();
		return;
	}
	//root.prettyPrintTo(Serial);

	JsonArray& bt_addressess = root["bt_addressess"];


	for(int i = 0; i < bt_addressess.size(); i++)
	{
		const char* bt_addressess_name = bt_addressess[i]["name"].as<char*>();
		const char* bt_addressess_address = bt_addressess[i]["address"];
		strcpy(settings.bt_addressess[i].address, bt_addressess_address);
		strcpy(settings.bt_addressess[i].name, bt_addressess_name);
	}

	int bluetoothAutoConnect = root["bluetoothAutoConnect"];
	settings.bluetoothAutoConnect = bluetoothAutoConnect;

	int mouseaccel0 = root["mouseaccel"][0];
	int mouseaccel1 = root["mouseaccel"][1];
	settings.mouseaccel[0] = mouseaccel0;
	settings.mouseaccel[1] = mouseaccel1;

// Close the file (File's destructor doesn't close the file)
	file.close();
}

// Saves the configuration to a file
void saveSettings() {
	// Delete existing file, otherwise the configuration is appended to the file
	SD.remove(settingsFile);
	// Open file for writing
	FsFile file = SD.open(settingsFile, FILE_WRITE);
	if (!file) {
		Serial.println(F("Failed to create file"));
		return;
	}


	DynamicJsonBuffer jsonBuffer;

	JsonObject& root = jsonBuffer.createObject();

	JsonArray& bt_addressess = root.createNestedArray("bt_addressess");

	for (auto& bt_addresses : settings.bt_addressess)
	{
		JsonObject& bt_addressess_obj = bt_addressess.createNestedObject();
		bt_addressess_obj["name"] = bt_addresses.name;
		bt_addressess_obj["address"] = bt_addresses.address;
	}

	root["bluetoothAutoConnect"] = settings.bluetoothAutoConnect;

	JsonArray& mouseaccel = root.createNestedArray("mouseaccel");
	mouseaccel.add(settings.mouseaccel[0]);
	mouseaccel.add(settings.mouseaccel[1]);


	// // Serialize JSON to file
	if (root.printTo(file) == 0) {
		Serial.println(F("Failed to write to file"));
	}

	// Close the file (File's destructor doesn't close the file)
	file.close();
}