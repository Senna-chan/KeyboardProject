/*
 Name:		ComboProcessorSetup.ino
 Created:	9/11/2018 9:01:15 PM
 Author:	Natsuki
*/
#define BT_PREFIX 			"BT_STAT%"
// the setup function runs once when you press reset or power the board

void readSerial2()
{
	delay(200);
	while (Serial2.available() > 0)
	{
		Serial.print((char)Serial2.read());
	}
	delay(250);
}

void setup() {
	Serial2.begin(115200);
	Serial.begin(115200);
	Serial.println("beginning to setup bluetooth module");
	delay(500);
	Serial2.print("$$$"); // Enter command mode
	readSerial2();
	Serial2.print("SF,1\n"); // Reset module
	readSerial2();
	Serial2.print("+\n"); // Sets up echo mode(All commands are printed back
	readSerial2();
	Serial2.print("SM,0\n"); // Sets mode to slave mode
	readSerial2();
	Serial2.print("SA,1\n"); // Sets authentication to SSP Keyboard/IO mode
	readSerial2();
	Serial2.print("SN,Adv. Keyboard\n"); // Sets name to what it says
	readSerial2();
	Serial2.print("SO,"); // Sets a command prefix
	Serial2.print(BT_PREFIX);
	Serial2.print("\n");
	readSerial2();
	Serial2.print("ST,255\n"); // Sets remote configuration timer to forever
	readSerial2();
	Serial2.print("S~,6\n"); // Setup module as HID
	readSerial2();
	Serial2.print("SH,0038\n"); // Setup Keyboard + Mouse with output reports on
	readSerial2();
	Serial2.print("R,1\n");
	readSerial2();
	Serial.println("Rebooting, waiting for 2 seconds");
	delay(2000);
	Serial.println ("Done setting up module. Printing settings");
	Serial2.print("$$$");
	readSerial2();
	Serial2.print("D\n");
	readSerial2();
	Serial2.print("E\n");
	readSerial2();
	Serial.println("Starting serial passthrough for additional setup things");
}

// the loop function runs over and over again until power down or reset
void loop()
{
	while (Serial.available() > 0) Serial2.print((char)Serial.read());
	while (Serial2.available() > 0) Serial.print((char)Serial2.read());
}
