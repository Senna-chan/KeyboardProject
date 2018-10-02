using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.Threading;

namespace BluetoothDebugger
{
    class ComWatcher
    {
        private SerialPort _comPort;
        private const byte _startByte = 0xFD;
        private bool beginSequence = false;
        private bool commandMode = false; // Activates when sending $$$
        public bool shouldStop = false;
        public ComWatcher(string comPort)
        {
            _comPort = new SerialPort(comPort, 115200, Parity.None, 8, StopBits.One);
            _comPort.Open();
            _comPort.DiscardInBuffer();
            new Thread(ParseReceivedData).Start();
        }

        private void ParseReceivedData()
        {
            var receivedByte = 0x00;
            var wasteByte = 0x00;
            Stopwatch stopwatch = new Stopwatch();
            while (!shouldStop)
            {
                if (_comPort.BytesToRead == 0) continue;
                stopwatch.Restart();
                receivedByte = _comPort.ReadByte();

                if (receivedByte == '$')
                {
                    commandMode = true;
                }

                if (commandMode)
                {
                    while (commandMode)
                    {
                        if (_comPort.ReadByte() == '$' && _comPort.ReadByte() == '$')
                        {
                            _comPort.Write("CMD\r\n");
                        }

                        var command = _comPort.ReadTo("\r\n");
                        if (command.StartsWith("C")) command = "C";
                        if (command.StartsWith("SN,")) command = "SN";
                        switch (command)
                        {
                            case "SN":
                            case "AW":
                                _comPort.Write("AOK\r\n");
                                break;
                            case "R,1":
                                _comPort.Write("Reboot!\r\n");
                                break;
                            case "---":
                                _comPort.Write("END\r\n");
                                commandMode = false;
                                break;
                            case "C":
                                _comPort.Write("TRYING\r\n");
                                commandMode = false;
                                break;
                            default:
                                _comPort.Write("AOK\r\n");
                                break;
                        }
                    }
                    continue;
                }

                if (receivedByte == _startByte)
                {
                    beginSequence = true;
                }

                if (!beginSequence) continue;

                receivedByte = _comPort.ReadByte();



                if (receivedByte == 0x9) // KeyboardReport
                {
                    wasteByte = _comPort.ReadByte(); // Descriptor
                    VariableStorage.KeyboardViewModel.Modifiers = _comPort.ReadByte().ToString();
                    wasteByte = _comPort.ReadByte(); // Nothing
                    VariableStorage.KeyboardViewModel.Key1 = _comPort.ReadByte().ToString();
                    VariableStorage.KeyboardViewModel.Key2 = _comPort.ReadByte().ToString();
                    VariableStorage.KeyboardViewModel.Key3 = _comPort.ReadByte().ToString();
                    VariableStorage.KeyboardViewModel.Key4 = _comPort.ReadByte().ToString();
                    VariableStorage.KeyboardViewModel.Key5 = _comPort.ReadByte().ToString();
                    VariableStorage.KeyboardViewModel.Key6 = _comPort.ReadByte().ToString();
                    beginSequence = false;
                }

                if (receivedByte == 0x5) // MouseReport
                {
                    wasteByte = _comPort.ReadByte(); // Descriptor
                    VariableStorage.MouseViewModel.MouseButtons = _comPort.ReadByte().ToString();
                    VariableStorage.MouseViewModel.MouseX = (sbyte) _comPort.ReadByte();
                    VariableStorage.MouseViewModel.MouseY = (sbyte) _comPort.ReadByte();
                    VariableStorage.MouseViewModel.Wheel = (sbyte) _comPort.ReadByte();
                    beginSequence = false;
                }
                _comPort.DiscardInBuffer();
                VariableStorage.MiscViewModel.ReceiveTime = stopwatch.ElapsedMilliseconds;
            }
        }
    }
}
