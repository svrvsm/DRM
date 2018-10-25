using System;
using System.Windows;
using System.Windows.Media.Imaging;
using System.Windows.Interop;

namespace DRM
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        const string FILTER = "Image Files(*.BMP;*.JPG;*.GIF;*.PNG)|*.BMP;*.JPG;*.GIF;*.PNG|All files (*.*)|*.*";
        
        public MainWindow()
        {
            InitializeComponent();
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            IntPtr mainHandle = new WindowInteropHelper(this).Handle;
            DWMWrapper.EnableDWM(mainHandle);
        }

        private void OnOpen(object sender, RoutedEventArgs e)
        {
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
