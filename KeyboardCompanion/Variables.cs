using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Configuration.ConfigurationSettings;

namespace KeyboardCompanion
{
    public static class Variables
    {
        public static SerialPort KeyBoardPort;
        public static String KeyboardVid = AppSettings.Get("KeyboardVID");
        public static String KeyboardPid = AppSettings.Get("KeyboardPID");
        public static bool KeyboardDetected = false;
    }
}
