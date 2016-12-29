using System;
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

public enum CaptureMode
{
    PrintWindow = 0,
    BitBlt = 1,
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
    public const string name = "uWindowCapture";

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugLogDelegate(string str);

    [DllImport(name, EntryPoint = "UwcInitialize")]
    public static extern void Initialize();
    [DllImport(name, EntryPoint = "UwcFinalize")]
    public static extern void Finalize();
    [DllImport(name, EntryPoint = "UwcSetDebugMode")]
    public static extern void SetDebugMode(DebugMode mode);
    [DllImport(name, EntryPoint = "UwcSetLogFunc")]
    public static extern void SetLogFunc(DebugLogDelegate func);
    [DllImport(name, EntryPoint = "UwcSetErrorFunc")]
    public static extern void SetErrorFunc(DebugLogDelegate func);
    [DllImport(name, EntryPoint = "UwcGetRenderEventFunc")]
    public static extern IntPtr GetRenderEventFunc();
    [DllImport(name, EntryPoint = "UwcRequestUpdateWindowList")]
    public static extern void RequestUpdateWindowList();
    [DllImport(name, EntryPoint = "UwcGetWindowCount")]
    public static extern int GetWindowCount();
    [DllImport(name, EntryPoint = "UwcGetWindowList")]
    public static extern IntPtr GetWindowList();
    [DllImport(name, EntryPoint = "UwcAddWindow")]
    public static extern int AddWindow(IntPtr hwnd);
    [DllImport(name, EntryPoint = "UwcRemoveWindow")]
    public static extern void RemoveWindow(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowVisible")]
    public static extern bool IsWindowVisible(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowHandle")]
    public static extern IntPtr GetWindowHandle(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowWidth")]
    public static extern int GetWindowWidth(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowHeight")]
    public static extern int GetWindowHeight(int id);
    [DllImport(name, EntryPoint = "UwcSetWindowTexturePtr")]
    public static extern void SetWindowTexturePtr(int id, IntPtr texturePtr);
    [DllImport(name, EntryPoint = "UwcSetWindowCaptureMode")]
    public static extern void SetWindowCaptureMode(int id, CaptureMode mode);
}

}