using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Interop;

// DRM == Digital rights manager
namespace DRM
{
    // DWM == Desktop Window Manager
    static class DWMWrapper
    {
        [DllImport("dwmapi.dll", EntryPoint = "DwmIsCompositionEnabled", PreserveSig = false)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool DwmIsCompositionEnabled();

        [DllImport("User32.dll", EntryPoint = "SetWindowDisplayAffinity")]
        public static extern bool SetWindowDisplayAffinity(IntPtr handle, uint flag);

        [DllImport("Hook.dll", EntryPoint = "SetWindowHandle")]
        public static extern bool SetWindowHandle(IntPtr handle);

        [DllImport("Hook.dll", EntryPoint = "InstallHook")]
        public static extern bool InstallHook();

        [DllImport("Hook.dll", EntryPoint = "UninstallHook")]
        public static extern bool UninstallHook();

        const uint WDA_MONITOR = 1;

        public static bool EnableDWM(IntPtr mainWindowHandle)
        {
            bool dwmEnabled = false;
            // Check to see if composition is Enabled
            if (Environment.OSVersion.Version.Major >= 6)
            { 
                dwmEnabled = DwmIsCompositionEnabled();
            }

            if (dwmEnabled)
            {
                dwmEnabled = SetWindowDisplayAffinity(mainWindowHandle, WDA_MONITOR);

                return dwmEnabled;
            }

            // Now the case where DWM is disabled.
            // On Vista dwm service can be turned off.
            // So manually hook prevent print screen and 
            // snipping tool screen capture.
            dwmEnabled = SetupHook(mainWindowHandle);

            return dwmEnabled;
        }

        public static bool SetupHook(IntPtr mainWindowHandle)
        {
            SetWindowHandle(mainWindowHandle);

            if (!InstallHook())
                return false;

            return true;
        }

        public static void ReleaseHook()
        {
            UninstallHook();
        }
    }
}
