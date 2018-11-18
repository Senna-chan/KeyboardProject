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
using KeyboardCompanion.Enums;

namespace KeyboardCompanion
{
    /// <summary>
    /// Interaction logic for DisplayAllMacros.xaml
    /// </summary>
    public partial class DisplayAllMacros : Window
    {
        public DisplayAllMacros(KeyMacro[,] keyMacros)
        {
            InitializeComponent();
            for (int i = 0; i < 9; i++) // Macro
            {

                TabItem root = new TabItem() { Header = $"Macro {i+1}"};
                Grid grid = new Grid();
                StackPanel rootStackPanel = new StackPanel();
                for (int j = 0; j < EditCreateMacro.macroKeyDepth; j++) // Macro Keys
                {
                    KeyMacro macro = keyMacros[i, j];

                    StackPanel macroPanel = new StackPanel(){Orientation = Orientation.Horizontal, Margin = new Thickness(0,20,0,20)};
                    if (macro.modifier == 0xff)
                    {
                        macroPanel.Children.Add(new TextBlock() { Text = $"ConsumerKey: {Enum.ToObject(typeof(ConsumerKeys), (macro.keys[0] << 8 | macro.keys[1]))}", Margin = new Thickness(10, 0, 10, 0) });
                    }
                    else { 
                        macroPanel.Children.Add(new CheckBox() { IsEnabled = false, IsChecked = (macro.modifier & (1 << 0)) != 0,Content="CTLR"});
                        macroPanel.Children.Add(new CheckBox() { IsEnabled = false, IsChecked = (macro.modifier & (1 << 1)) != 0,Content="SHIFT"});
                        macroPanel.Children.Add(new CheckBox() { IsEnabled = false, IsChecked = (macro.modifier & (1 << 2)) != 0,Content="ALT"});
                        macroPanel.Children.Add(new CheckBox() { IsEnabled = false, IsChecked = (macro.modifier & (1 << 3)) != 0,Content="WIN"});
                        for (int k = 0; k < 6; k++) // Key
                        {
                            macroPanel.Children.Add(new TextBlock() {Text = $"Key {k}: {Enum.ToObject(typeof(KeyboardKeys),macro.keys[k])}", Margin = new Thickness(10,0,10,0) });
                        }
                    }
                    rootStackPanel.Children.Add(macroPanel);
                }

                grid.Children.Add(rootStackPanel);
                root.Content = grid;
                TabControl.Items.Add(root);
            }
        }
    }
}
