using System.Runtime.InteropServices;

#pragma warning disable 114, 465

namespace uWindowCapture
{

public enum DebugMode
{
    None = 0,
    File = 1,
    UnityLog = 2, /* currently has bug when app exits. */
}

public static class Lib
{
    [DllImport("uWindowCapture")]
    public static extern void SetWindowTexture(System.IntPtr texturePtr);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugLogDelegate(string str);

    [DllImport("uWindowCapture")]
    public static extern void Initialize();
    [DllImport("uWindowCapture")]
    public static extern void Finalize();
    [DllImport("uWindowCapture")]
    public static extern void SetDebugMode(DebugMode mode);
    [DllImport("uWindowCapture")]
    public static extern void SetLogFunc(DebugLogDelegate func);
    [DllImport("uWindowCapture")]
    public static extern void SetErrorFunc(DebugLogDelegate func);
    [DllImport("uWindowCapture")]
    public static extern System.IntPtr GetRenderEventFunc();
    [DllImport("uWindowCapture")]
    public static extern System.IntPtr GetWindowTexture(ref int width, ref int height);
    [DllImport("uWindowCapture")]
    public static extern int GetWidth();
    [DllImport("uWindowCapture")]
    public static extern int GetHeight();
}

}