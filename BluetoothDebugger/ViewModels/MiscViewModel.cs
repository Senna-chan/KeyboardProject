using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BluetoothDebugger.ViewModels
{
    public class MiscViewModel : ObservableObject
    {
        private long _receiveTime;
        private int _receiveBufferSize;

        public long ReceiveTime
        {
            get => _receiveTime;
            set
            {
                _receiveTime = value;
                RaisePropertyChangedEvent("ReceiveTime");
            }
        }
        public int ReceiveBufferSize
        {
            get => _receiveBufferSize;
            set
            {
                _receiveBufferSize = value;
                RaisePropertyChangedEvent("ReceiveBuffer");
            }
        }
    }
}
