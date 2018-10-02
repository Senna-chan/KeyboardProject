// 
// 
// 

#include "DuckyReadWriter.h"
#include <USBHID.h>
#include "SdFs.h"
#include "HIDFunctions.h"
#include "Variables.h"




void ReadDucky(String fileName) {
	FsFile myFile = SD.open("ducky"+fileName);
	if (myFile) {

		String line = "";
		while (myFile.available()) {
			char m = myFile.read();
			if (m == '\n') {
				Line(line);
				line = "";
			}
			else if ((int)m != 13)
			{
				line += m;
			}
		}
		Line(line);

		myFile.close();
	}
	else {
	}
}

void Line(String l)
{
	int space_1 = l.indexOf(" ");
	if (space_1 == -1)
	{
		Press(l);
	}
	else if (l.substring(0, space_1) == "STRING")
	{
		printKeyboardString(l.substring(space_1 + 1));
	}
	else if (l.substring(0, space_1) == "DELAY")
	{
		int delaytime = l.substring(space_1 + 1).toInt();
		delay(delaytime);
	}
	else if (l.substring(0, space_1) == "REM") {}
	else
	{
		String remain = l;

		while (remain.length() > 0)
		{
			int latest_space = remain.indexOf(" ");
			if (latest_space == -1)
			{
				Press(remain);
				remain = "";
			}
			else
			{
				Press(remain.substring(0, latest_space));
				remain = remain.substring(latest_space + 1);
			}
			delay(5);
		}
	}
	releaseAllKeyboardKeys();
}

void Press(String b)
{
	if (b.length() == 1)
	{
		char c = b[0];
		pressKeyboardKey(c);
	}
	else if (b.equals("ENTER"))
	{
		pressKeyboardKey(KEY_RETURN);
	}
	else if (b.equals("CTRL"))
	{
		pressKeyboardKey(KEY_LEFT_CTRL);
	}
	else if (b.equals("SHIFT"))
	{
		pressKeyboardKey(KEY_LEFT_SHIFT);
	}
	else if (b.equals("ALT"))
	{
		pressKeyboardKey(KEY_LEFT_ALT);
	}
	else if (b.equals("GUI"))
	{
		pressKeyboardKey(KEY_LEFT_GUI);
	}
	else if (b.equals("UP") || b.equals("UPARROW"))
	{
		pressKeyboardKey(KEY_UP_ARROW);
	}
	else if (b.equals("DOWN") || b.equals("DOWNARROW"))
	{
		pressKeyboardKey(KEY_DOWN_ARROW);
	}
	else if (b.equals("LEFT") || b.equals("LEFTARROW"))
	{
		pressKeyboardKey(KEY_LEFT_ARROW);
	}
	else if (b.equals("RIGHT") || b.equals("RIGHTARROW"))
	{
		pressKeyboardKey(KEY_RIGHT_ARROW);
	}
	else if (b.equals("DELETE"))
	{
		pressKeyboardKey(KEY_DELETE);
	}
	else if (b.equals("PAGEUP"))
	{
		pressKeyboardKey(KEY_PAGE_UP);
	}
	else if (b.equals("PAGEDOWN"))
	{
		pressKeyboardKey(KEY_PAGE_DOWN);
	}
	else if (b.equals("HOME"))
	{
		pressKeyboardKey(KEY_HOME);
	}
	else if (b.equals("ESC"))
	{
		pressKeyboardKey(KEY_ESC);
	}
	else if (b.equals("INSERT"))
	{
		pressKeyboardKey(KEY_INSERT);
	}
	else if (b.equals("TAB"))
	{
		pressKeyboardKey(KEY_TAB);
	}
	else if (b.equals("END"))
	{
		pressKeyboardKey(KEY_END);
	}
	else if (b.equals("CAPSLOCK"))
	{
		pressKeyboardKey(KEY_CAPS_LOCK);
	}
	else if (b.equals("F1"))
	{
		pressKeyboardKey(KEY_F1);
	}
	else if (b.equals("F2"))
	{
		pressKeyboardKey(KEY_F2);
	}
	else if (b.equals("F3"))
	{
		pressKeyboardKey(KEY_F3);
	}
	else if (b.equals("F4"))
	{
		pressKeyboardKey(KEY_F4);
	}
	else if (b.equals("F5"))
	{
		pressKeyboardKey(KEY_F5);
	}
	else if (b.equals("F6"))
	{
		pressKeyboardKey(KEY_F6);
	}
	else if (b.equals("F7"))
	{
		pressKeyboardKey(KEY_F7);
	}
	else if (b.equals("F8"))
	{
		pressKeyboardKey(KEY_F8);
	}
	else if (b.equals("F9"))
	{
		pressKeyboardKey(KEY_F9);
	}
	else if (b.equals("F10"))
	{
		pressKeyboardKey(KEY_F10);
	}
	else if (b.equals("F11"))
	{
		pressKeyboardKey(KEY_F11);
	}
	else if (b.equals("F12"))
	{
		pressKeyboardKey(KEY_F12);
	}
}
