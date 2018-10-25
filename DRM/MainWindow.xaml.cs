using System;
using System.Windows;
using System.Windows.Media.Imaging;
using System.Windows.Interop;

namespace DRM
{
    public partial class MainWindow : Window
    {
        const string FILTER = "Image Files(*.BMP;*.JPG;*.GIF;*.PNG)|*.BMP;*.JPG;*.GIF;*.PNG|All files (*.*)|*.*";
        const uint WM_SNIP_ALERT = 1025;
        const uint SNIP_ACTIVE = 1;

        string imagePath_;
   
        public MainWindow()
        {
            InitializeComponent();
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            try
            {
                IntPtr windowHandle = (new WindowInteropHelper(this)).Handle;
                HwndSource src = HwndSource.FromHwnd(windowHandle);
                src.AddHook(new HwndSourceHook(WndProc));
                DWMWrapper.EnableDWM(windowHandle);
            }
            catch
            {
                Console.WriteLine("Exception caught");
            }
        }

        private IntPtr WndProc(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg != WM_SNIP_ALERT)
                return IntPtr.Zero;

            // If snip tool is active then we need to prevent the 
            // action

            // WM_SNIP_ALERT message is posted by the hook.dll, when it intercepts snipping tool
            // WndProc get message 
            int wp = wParam.ToInt32();
            if (wp == SNIP_ACTIVE)
            {
                if (Img1.Source != null)
                {
                    Img1.Source = null;
                }
                return IntPtr.Zero;
            }

            if (string.IsNullOrEmpty(imagePath_))
                return IntPtr.Zero;

            LoadImage();
            Img1.InvalidateVisual();
            // TODO: The image refresh doesn't happen and another click is required.

            return IntPtr.Zero;
        }

        private void LoadImage()
        {
            try
            {
                BitmapImage bi = new BitmapImage();
                bi.BeginInit();
                bi.CacheOption = BitmapCacheOption.OnLoad;
                bi.CreateOptions = BitmapCreateOptions.IgnoreImageCache;
                bi.UriSource = new Uri(imagePath_);
                bi.EndInit();

                Img1.Source = null;
                Img1.Source = bi;
            }
            catch
            {
                MessageBox.Show("Invalid File");
                Console.WriteLine("Exception caught");
            }
        }

        private void MainWindow_Closing(object sender, EventArgs e)
        {
            DWMWrapper.ReleaseHook();
            IntPtr windowHandle = (new WindowInteropHelper(this)).Handle;
            HwndSource src = HwndSource.FromHwnd(windowHandle);
            src.RemoveHook(new HwndSourceHook(this.WndProc));
        }

        private void OnOpen(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.OpenFileDialog openFileDialog = new System.Windows.Forms.OpenFileDialog();
            openFileDialog.Filter = FILTER;
            if (openFileDialog.ShowDialog() != System.Windows.Forms.DialogResult.OK)
                return;
            imagePath_ = openFileDialog.FileName;
            LoadImage();
        }
    }
}
