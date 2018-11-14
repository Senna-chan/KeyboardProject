using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Markup;
using KeyboardCompanion.Enums;
using Microsoft.Win32;

namespace KeyboardCompanion
{
    public class KeyMacro
    {
        public byte modifier { get; set; }
        public byte[] keys { get; set; }

        public KeyMacro()
        {
            modifier = 0;
            keys = new byte[6] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        }

        public byte[] getBytes()
        {
            var bytes = new List<byte>();
            bytes.Add(modifier);
            bytes.AddRange(keys);
            return bytes.ToArray();
        }
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private String fileLocation = "";
        private KeyMacro[,] keyMacros = new KeyMacro[9,9];
        private int keyMacroComboPos = 0;
        public MainWindow()
        {
            InitializeComponent();
            for (int i = 0; i < 9; i++) // Macro
            {
                for (int j = 0; j < 9; j++) // Macro Keys
                {
                    keyMacros[i, j] = new KeyMacro();
                }
            }

            macroKeyCombo.DropDownClosed += (sender, args) =>
                keyMacroComboPos = Convert.ToInt32(((ComboBox) sender).Text) - 1;

            cbxKey1.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey2.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey3.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey4.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey5.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey6.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();

            cbxKey1.SelectedIndex = 0x00;   
            cbxKey2.SelectedIndex = 0x00;
            cbxKey3.SelectedIndex = 0x00;
            cbxKey4.SelectedIndex = 0x00;
            cbxKey5.SelectedIndex = 0x00;
            cbxKey6.SelectedIndex = 0x00;

            cbxKey1.DropDownClosed += keyValueChanged;
            cbxKey2.DropDownClosed += keyValueChanged;
            cbxKey3.DropDownClosed += keyValueChanged;
            cbxKey4.DropDownClosed += keyValueChanged;
            cbxKey5.DropDownClosed += keyValueChanged;
            cbxKey6.DropDownClosed += keyValueChanged;

            saveMacro1.Click += saveMacro;
            saveMacro2.Click += saveMacro;
            saveMacro3.Click += saveMacro;
            saveMacro4.Click += saveMacro;
            saveMacro5.Click += saveMacro;
            saveMacro6.Click += saveMacro;
            saveMacro7.Click += saveMacro;
            saveMacro8.Click += saveMacro;
            saveMacro9.Click += saveMacro;

            getMacro1.Click += getMacro;
            getMacro2.Click += getMacro;
            getMacro3.Click += getMacro;
            getMacro4.Click += getMacro;
            getMacro5.Click += getMacro;
            getMacro6.Click += getMacro;
            getMacro7.Click += getMacro;
            getMacro8.Click += getMacro;
            getMacro9.Click += getMacro;
        }

        private void openFileDialog(bool openDialog)
        {
            FileDialog fileDialog;
            if (openDialog)
            {
                fileDialog = new OpenFileDialog();
                fileDialog.Title = "Select file";
            }
            else
            {
                fileDialog = new SaveFileDialog();
            }
            
            
            fileDialog.CheckFileExists = false;
            fileDialog.Filter = "Keyboard Macros (macros)|macros";
            if (fileDialog.ShowDialog() == true)
            {
                fileLocation = fileDialog.FileName;
            }

            using (StreamReader reader = new StreamReader(fileLocation))
            {
                for (int i = 0; i < 9; i++) // Macro
                {
                    for (int j = 0; j < 9; j++) // Macro Keys
                    {
                        try
                        {
                            keyMacros[i, j].modifier = (byte) reader.Read();
                            for (int k = 0; k < 6; k++) // Key
                            {
                                keyMacros[i, j].keys[k] = (byte) reader.Read();
                            }
                        }
                        catch (Exception ex)
                        {
                            keyMacros[i, j] = new KeyMacro();
                        }
                    }
                }
            }
        }

        private void keyValueChanged(object sender, EventArgs e)
        {
            ComboBox cbx = (ComboBox) sender;
            Console.WriteLine($"{cbx.Name}:{cbx.SelectedValue}({(int)cbx.SelectedValue})");
        }

        private void saveMacro(object sender, EventArgs e)
        {
            int macroPos = int.Parse(((Button) sender).Name.Substring(9)) - 1;
            var value = 0 | (rchecked(chbCtrl) << 0) | (rchecked(chbShift) << 1) |
                        (rchecked(chbAlt) << 2) | (rchecked(chbWin) << 3);
            var keyMacro = new KeyMacro
            {
                modifier = (byte) (value),
                keys =
                {
                    [0] = (byte)((int)cbxKey1.SelectedValue),
                    [1] = (byte)((int)cbxKey2.SelectedValue),
                    [2] = (byte)((int)cbxKey3.SelectedValue),
                    [3] = (byte)((int)cbxKey4.SelectedValue),
                    [4] = (byte)((int)cbxKey5.SelectedValue),
                    [5] = (byte)((int)cbxKey6.SelectedValue)
                }
            };

            keyMacros[keyMacroComboPos, macroPos] = keyMacro;

        }

        private void getMacro(object sender, EventArgs e)
        {
            int macroPos = int.Parse(((Button)sender).Name.Substring(8)) - 1;
            var keyMacro = keyMacros[keyMacroComboPos, macroPos];
            chbCtrl.IsChecked = (keyMacro.modifier & (1 << 0)) != 0;
            chbShift.IsChecked = (keyMacro.modifier & (1 << 1)) != 0;
            chbAlt.IsChecked = (keyMacro.modifier & (1 << 2)) != 0;
            chbWin.IsChecked = (keyMacro.modifier & (1 << 3)) != 0;
            cbxKey1.SelectedIndex = keyMacro.keys[0] > 0 ? keyMacro.keys[0] - 3 : 0;
            cbxKey2.SelectedIndex = keyMacro.keys[1] > 0 ? keyMacro.keys[1] - 3 : 0;
            cbxKey3.SelectedIndex = keyMacro.keys[2] > 0 ? keyMacro.keys[2] - 3 : 0;
            cbxKey4.SelectedIndex = keyMacro.keys[3] > 0 ? keyMacro.keys[3] - 3 : 0;
            cbxKey5.SelectedIndex = keyMacro.keys[4] > 0 ? keyMacro.keys[4] - 3 : 0;
            cbxKey6.SelectedIndex = keyMacro.keys[5] > 0 ? keyMacro.keys[5] - 3 : 0;

        }

        private int rchecked(CheckBox check)
        {
            return check.IsChecked == true ? 1 : 0;
        }

        private void SaveFile_Click(object sender, RoutedEventArgs e)
        {
            if(string.IsNullOrEmpty(fileLocation)) openFileDialog(false);
            using (FileStream fs = new FileStream(fileLocation, FileMode.OpenOrCreate))
            {
                for (int i = 0; i < 9; i++) // Macro
                {
                    for (int j = 0; j < 9; j++) // Macro Keys
                    {
                        fs.Write(keyMacros[i,j].getBytes(),0,7);
                    }
                }
            }
        }

        private void DisplayAll_OnClick(object sender, RoutedEventArgs e)
        {
            new DisplayAllMacros(keyMacros).Show();
        }

        private void OpenFile_OnClick(object sender, RoutedEventArgs e)
        {
            openFileDialog(true);
        }
    }
}
