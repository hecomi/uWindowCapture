using System;
using System.Text;
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

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)]
public struct WindowInfo
{
    [MarshalAs(UnmanagedType.I8)]
    public IntPtr handle;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst=256)]
    public string title;
}

public static class Lib
{
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
    public static extern IntPtr GetRenderEventFunc();
    [DllImport("uWindowCapture")]
    public static extern void RequestUpdateWindowList();
    [DllImport("uWindowCapture")]
    public static extern int GetWindowCount();
    [DllImport("uWindowCapture")]
    public static extern IntPtr GetWindowList();
    [DllImport("uWindowCapture")]
    public static extern int AddWindow(IntPtr hwnd);
    [DllImport("uWindowCapture")]
    public static extern void RemoveWindow(int id);
    [DllImport("uWindowCapture")]
    public static extern bool IsAlive(int id);
    [DllImport("uWindowCapture")]
    public static extern IntPtr GetHandle(int id);
    [DllImport("uWindowCapture")]
    public static extern string GetTitle(int id, StringBuilder buf, int len);
    [DllImport("uWindowCapture")]
    public static extern int GetWidth(int id);
    [DllImport("uWindowCapture")]
    public static extern int GetHeight(int id);
    [DllImport("uWindowCapture")]
    public static extern void SetTexturePtr(int id, IntPtr texturePtr);

    public static string GetTitle(int id)
    {
        var buf = new StringBuilder(256);
        GetTitle(id, buf, buf.Capacity);
        return buf.ToString();
    }
}

}