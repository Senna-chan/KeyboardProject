using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BluetoothDebugger.Helpers;

namespace BluetoothDebugger.ViewModels
{
    public class MouseViewModel : ObservableObject
    {
        private sbyte _mouseX;
        private sbyte _mouseY;
        private byte _mouseButtons;
        private sbyte _wheel;

        public sbyte MouseX
        {
            get => _mouseX;
            set
            {
                _mouseX = value; 
                RaisePropertyChangedEvent("MouseX");
            }
        }

        public sbyte MouseY
        {
            get => _mouseY;
            set
            {
                _mouseY = value; 
                RaisePropertyChangedEvent("MouseY");
            }
        }

        public string MouseButtons
        {
            get
            {
                if (_mouseButtons == 0) return "";
                var returnVal = "";
                if (_mouseButtons.GetBit(0))
                {
                    returnVal += "LEFT, ";
                }
                if (_mouseButtons.GetBit(1))
                {
                    returnVal += "RIGHT, ";
                }
                if (_mouseButtons.GetBit(2))
                {
                    returnVal += "MIDDLE, ";
                }

                if (returnVal.Length > 0)
                {
                    returnVal = returnVal.Substring(0, returnVal.Length - 2);
                }
                return returnVal;
            }
            set
            {
                _mouseButtons = byte.Parse(value);
                RaisePropertyChangedEvent("MouseButtons");
            }
        }

        public sbyte Wheel
        {
            get => _wheel;
            set
            {
                _wheel = value;
                RaisePropertyChangedEvent("Wheel");
            }
        }
    }
}
