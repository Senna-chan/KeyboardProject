using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BluetoothDebugger.Helpers
{
    public static class Helpers
    {
        public static bool GetBit(this byte val, int pos)
        {
            val -= 1;
            return (1 == ((val >> pos) & 1));
        }
        public static bool GetBit(this int val, int pos)
        {
            val -= 1;
            return (1 == ((val >> pos) & 1));
        }
    }
}
