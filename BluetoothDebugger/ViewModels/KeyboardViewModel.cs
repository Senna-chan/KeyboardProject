using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BluetoothDebugger.Helpers;

namespace BluetoothDebugger.ViewModels
{
    public class KeyboardViewModel : ObservableObject
    {

        private byte _modifiers;
        private int _key1 = 0;
        private int _key2 = 0;
        private int _key3 = 0;
        private int _key4 = 0;
        private int _key5 = 0;
        private int _key6 = 0;

        public string Modifiers
        {
            get
            {
                if (_modifiers == 0) return "";
                string returnVal = "";
                if (_modifiers.GetBit(1))
                {
                    returnVal += "LEFT_CTRL, ";
                }
                if (_modifiers.GetBit(2))
                {
                    returnVal += "LEFT_SHIFT, ";
                }
                if (_modifiers.GetBit(3))
                {
                    returnVal += "LEFT_ALT, ";
                }
                if (_modifiers.GetBit(4))
                {
                    returnVal += "LEFT_GUI, ";
                }
                if (_modifiers.GetBit(5))
                {
                    returnVal += "RIGHT_CTRL, ";
                }
                if (_modifiers.GetBit(6))
                {
                    returnVal += "RIGHT_SHIFT, ";
                }
                if (_modifiers.GetBit(7))
                {
                    returnVal += "RIGHT_ALT, ";
                }
                if (_modifiers.GetBit(8))
                {
                    returnVal += "RIGHT_GUI, ";
                }

                if (returnVal.Length > 0)
                {
                    returnVal = returnVal.Substring(0, returnVal.Length - 2);
                }
                return returnVal;
            }
            set
            {
                _modifiers = byte.Parse(value);
                RaisePropertyChangedEvent("Modifiers");
            }
        }



        public string Key1
        {
            get => Enum.GetName(typeof(KeyCodes), _key1);
            set
            {
                _key1 = int.Parse(value);
                RaisePropertyChangedEvent("Key1");
            }
        }


        public string Key2
        {
            get => Enum.GetName(typeof(KeyCodes), _key2);
            set
            {
                _key2 = int.Parse(value);
                RaisePropertyChangedEvent("Key2");
            }
        }


        public string Key3
        {
            get => Enum.GetName(typeof(KeyCodes), _key3);
            set
            {
                _key3 = int.Parse(value);
                RaisePropertyChangedEvent("Key3");
            }
        }


        public string Key4
        {
            get => Enum.GetName(typeof(KeyCodes), _key4);
            set
            {
                _key4 = int.Parse(value);
                RaisePropertyChangedEvent("Key4");
            }
        }


        public string Key5
        {
            get => Enum.GetName(typeof(KeyCodes), _key5);
            set
            {
                _key5 = int.Parse(value);
                RaisePropertyChangedEvent("Key5");
            }
        }


        public string Key6
        {
            get => Enum.GetName(typeof(KeyCodes), _key6);
            set
            {
                _key6 = int.Parse(value);
                RaisePropertyChangedEvent("Key6");
            }
        }
    }
}
