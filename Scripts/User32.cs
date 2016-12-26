using System.Runtime.InteropServices;

namespace uWindowCapture
{

public static class User32
{
    [DllImport("user32.dll")]
    public static extern System.IntPtr GetForegroundWindow();
}

}