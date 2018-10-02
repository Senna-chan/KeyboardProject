using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BluetoothDebugger.ViewModels;

namespace BluetoothDebugger
{
    class VariableStorage
    {
        public static string ComPort = "";
        public static KeyboardViewModel KeyboardViewModel = new KeyboardViewModel();
        public static MouseViewModel MouseViewModel = new MouseViewModel();
        public static MiscViewModel MiscViewModel = new MiscViewModel();
    }
}
