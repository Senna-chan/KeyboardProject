using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.IO.Ports;

namespace BluetoothDebugger
{
    /// <summary>
    /// Interaction logic for SelectComPort.xaml
    /// </summary>
    public partial class SelectComPort : Window
    {
        public SelectComPort()
        {
            InitializeComponent();
            var comports = SerialPort.GetPortNames();
            foreach (var comport in comports)
            {
                ListBox.Items.Add(comport);
            }
        }
        

        private void SelectedComPort(object sender, MouseButtonEventArgs e)
        {
            if (ListBox.SelectedItem == null) return;
            VariableStorage.ComPort = ListBox.SelectedItem.ToString();
            var mainWindow = new MainWindow();
            this.Close();
            mainWindow.Show();
        }
    }
}
