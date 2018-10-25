using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
using System.Windows;
//using System.Windows.Controls;
//using System.Windows.Data;
//using System.Windows.Documents;
//using System.Windows.Input;
//using System.Windows.Media;
using System.Windows.Media.Imaging;
//using System.Windows.Navigation;
//using System.Windows.Shapes;
//using System.Windows.Forms;
using System.Windows.Interop;

namespace DRM
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const string FILTER = "Image Files(*.BMP;*.JPG;*.GIF;*.PNG)|*.BMP;*.JPG;*.GIF;*.PNG|All files (*.*)|*.*";
        bool dwmEnableAttempted_ = false;
        
        public MainWindow()
        {
            InitializeComponent();
        }

        private void OnOpen(object sender, RoutedEventArgs e)
        {
            if (!dwmEnableAttempted_)
            {
                IntPtr mainHandle = new WindowInteropHelper(this).Handle;
                DWMWrapper.EnableDWM(mainHandle);
                dwmEnableAttempted_ = true;
            }
            
            System.Windows.Forms.OpenFileDialog openFileDialog = new System.Windows.Forms.OpenFileDialog();
            openFileDialog.Filter = FILTER; 
            if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    Img1.Source = new BitmapImage(new Uri(openFileDialog.FileName));
                }
                catch
                {
                    MessageBox.Show("Invalid File");
                }
            }
        }
    }
}
