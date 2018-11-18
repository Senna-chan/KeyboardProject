// SettingsHelper.h

#ifndef _SETTINGSHELPER_h
#define _SETTINGSHELPER_h

#include <Arduino.h>
#include "Variables.h"

void loadSettings();
void saveSettings();
ConnectionConfig getConfigByName(String name);
ConnectionConfig getConfigByBTAddress(String btaddress);
void saveConnectionConfig(ConnectionConfig conConf);
ConnectionConfig createConnectionConfig();

#endif

