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
using System.Windows.Navigation;
using System.Windows.Shapes;
using BluetoothDebugger.ViewModels;

namespace BluetoothDebugger
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private ComWatcher comWatcher;
        public MainWindow()
        {
            InitializeComponent();
            VariableStorage.KeyboardViewModel = (KeyboardViewModel) Keyboard.DataContext;
            VariableStorage.MouseViewModel = (MouseViewModel) Mouse.DataContext;
            VariableStorage.MiscViewModel = (MiscViewModel) Misc.DataContext;
            comWatcher = new ComWatcher(VariableStorage.ComPort);
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            comWatcher.shouldStop = true;
            Application.Current.Shutdown();
        }
    }
}
