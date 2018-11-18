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
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class EditCreateMacro : Window
    {
        private String fileLocation = "";
        private int keyMacroComboPos = 0;
        public static int macroKeyDepth = 4;
        private KeyMacro[,] keyMacros = new KeyMacro[9, macroKeyDepth];
        public EditCreateMacro()
        {
            InitializeComponent();
            for (int i = 1; i <= macroKeyDepth; i++)
            {
                var saveButton = new Button {Name = "saveMacro" + i, Margin = new Thickness(0, 5, 0, 5), Content="Save "+i};
                saveButton.Click += SaveMacro;
                var getButton = new Button {Name = "getMacro" + i, Margin = new Thickness(0, 5, 0, 5), Content="Get "+i};
                getButton.Click += GetMacro;
                savePanel.Children.Add(saveButton);
                getPanel.Children.Add(getButton);
//                            <Button x:Name="saveMacro1" Content="Save 1" Margin="0,5" Click="SaveMacro"></Button>
//                            <Button x:Name="getMacro1" Content="Get 1" Margin="0,5" Click="GetMacro"></Button>
            }
            for (int i = 0; i < 9; i++) // Macro
            {
                for (int j = 0; j < macroKeyDepth; j++) // Macro Keys
                {
                    keyMacros[i, j] = new KeyMacro();
                }
            }
            macroKeyCombo.SelectedIndex = 0;
            macroKeyCombo.DropDownClosed += (sender, args) =>
                keyMacroComboPos = Convert.ToInt32(((ComboBox) sender).Text) - 1;

            cbxKey1.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey2.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey3.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey4.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey5.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();
            cbxKey6.ItemsSource = Enum.GetValues(typeof(KeyboardKeys)).Cast<KeyboardKeys>();

            cbxKeyC.ItemsSource = Enum.GetValues(typeof(ConsumerKeys)).Cast<ConsumerKeys>();

            cbxKey1.SelectedIndex = 0x00;
            cbxKey2.SelectedIndex = 0x00;
            cbxKey3.SelectedIndex = 0x00;
            cbxKey4.SelectedIndex = 0x00;
            cbxKey5.SelectedIndex = 0x00;
            cbxKey6.SelectedIndex = 0x00;

            cbxKeyC.SelectedIndex = 0x00;
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

            if (!File.Exists(fileLocation)) File.Create(fileLocation).Close();

            using (StreamReader reader = new StreamReader(fileLocation))
            {
                for (int i = 0; i < 9; i++) // Macro
                {
                    for (int j = 0; j < macroKeyDepth; j++) // Macro Keys
                    {
                        try
                        {
                            keyMacros[i, j].modifier = (byte) reader.BaseStream.ReadByte();
                            for (int k = 0; k < 6; k++) // Key
                            {
                                keyMacros[i, j].keys[k] = (byte)reader.BaseStream.ReadByte();
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

        private void SaveMacro(object sender, EventArgs e)
        {
            int macroPos = int.Parse(((Button) sender).Name.Substring(9)) - 1;
            if (rchecked(chbConsumer) == 1)
            {
                var keyMacro = new KeyMacro
                {
                    modifier = (byte)(0xff),
                    keys =
                    {
                        [0] = (byte) ((int) cbxKeyC.SelectedValue >> 8),
                        [1] = (byte) ((int) cbxKeyC.SelectedValue),
                        [2] = (byte) 0x00,
                        [3] = (byte) 0x00,
                        [4] = (byte) 0x00,
                        [5] = (byte) 0x00
                    }
                };
                keyMacros[keyMacroComboPos, macroPos] = keyMacro;
            }
            else
            {
                var value = 0 | (rchecked(chbCtrl) << 0) | (rchecked(chbShift) << 1) |
                            (rchecked(chbAlt) << 2) | (rchecked(chbWin) << 3);
                var keyMacro = new KeyMacro
                {
                    modifier = (byte) (value),
                    keys =
                    {
                        [0] = (byte) ((int) cbxKey1.SelectedValue),
                        [1] = (byte) ((int) cbxKey2.SelectedValue),
                        [2] = (byte) ((int) cbxKey3.SelectedValue),
                        [3] = (byte) ((int) cbxKey4.SelectedValue),
                        [4] = (byte) ((int) cbxKey5.SelectedValue),
                        [5] = (byte) ((int) cbxKey6.SelectedValue)
                    }
                };
                keyMacros[keyMacroComboPos, macroPos] = keyMacro;
            }
        }

        private void GetMacro(object sender, EventArgs e)
        {
            int macroPos = int.Parse(((Button)sender).Name.Substring(8)) - 1;
            var keyMacro = keyMacros[keyMacroComboPos, macroPos];
            if (keyMacro.modifier == 0xff)
            {
                chbConsumer.IsChecked = true;
                cbxKeyC.SelectedValue = Enum.ToObject(typeof(ConsumerKeys),(keyMacro.keys[0] << 8 | keyMacro.keys[1]));
                cbxKey1.SelectedIndex = 0;
                cbxKey2.SelectedIndex = 0;
                cbxKey3.SelectedIndex = 0;
                cbxKey4.SelectedIndex = 0;
                cbxKey5.SelectedIndex = 0;
                cbxKey6.SelectedIndex = 0;
                chbCtrl.IsChecked =       false;
                    chbShift.IsChecked=   false;
                chbAlt.IsChecked =        false;
                chbWin.IsChecked = false;
            }
            else
            {
                chbConsumer.IsChecked = false;
                chbCtrl.IsChecked = (keyMacro.modifier & (1 << 0)) != 0;
                chbShift.IsChecked = (keyMacro.modifier & (1 << 1)) != 0;
                chbAlt.IsChecked = (keyMacro.modifier & (1 << 2)) != 0;
                chbWin.IsChecked = (keyMacro.modifier & (1 << 3)) != 0;
                cbxKey1.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[0]);
                cbxKey2.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[1]);
                cbxKey3.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[2]);
                cbxKey4.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[3]);
                cbxKey5.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[4]);
                cbxKey6.SelectedValue = Enum.ToObject(typeof(KeyboardKeys), keyMacro.keys[5]);
                cbxKeyC.SelectedIndex = 0;
            }
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
                    for (int j = 0; j < macroKeyDepth; j++) // Macro Keys
                    {
                        fs.WriteByte(keyMacros[i, j].modifier);
                        for (int k = 0; k < 6; k++)
                        {
                            fs.WriteByte(keyMacros[i, j].keys[k]);
                        }
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

        private void NewFile_OnClick(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(fileLocation)) openFileDialog(false);
            for (int i = 0; i < macroKeyDepth; i++) // Macro
            {
                for (int j = 0; j < macroKeyDepth; j++) // Macro Keys
                {
                    keyMacros[i, j] = new KeyMacro();
                }
            }
        }
    }
}
