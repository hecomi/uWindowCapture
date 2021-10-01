using UnityEngine;
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
    None = -1,
    PrintWindow = 0,
    BitBlt = 1,
    WindowsGraphicsCapture = 2,
    Auto = 3,
}

public enum CapturePriority
{
    Auto = -1,
    High = 0,
    Middle = 1,
    Low = 2,
}

public enum MessageType
{
    None = -1,
    WindowAdded = 0,
    WindowRemoved = 1,
    WindowCaptured = 2,
    WindowSizeChanged = 3,
    IconCaptured = 4,
    CursorCaptured = 5,
    Error = 1000,
    TextureNullError = 1001,
    TextureSizeError = 1002,
}

[StructLayout(LayoutKind.Sequential)]
public struct Message
{
    [MarshalAs(UnmanagedType.I4)]
    public MessageType type;
    [MarshalAs(UnmanagedType.I4)]
    public int windowId;
    [MarshalAs(UnmanagedType.I8)]
    public IntPtr userData;
}

[StructLayout(LayoutKind.Sequential)]
public struct Point
{
    [MarshalAs(UnmanagedType.I4)]
    public int x;
    [MarshalAs(UnmanagedType.I4)]
    public int y;
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
    [DllImport(name, EntryPoint = "UwcUpdate")]
    public static extern void Update(float dt);
    [DllImport(name, EntryPoint = "UwcGetMessageCount")]
    private static extern int GetMessageCount();
    [DllImport(name, EntryPoint = "UwcGetMessages")]
    private static extern IntPtr GetMessages_Internal();
    [DllImport(name, EntryPoint = "UwcClearMessages")]
    private static extern void ClearMessages();
    [DllImport(name, EntryPoint = "UwcCheckWindowExistence")]
    public static extern bool CheckWindowExistence(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowHandle")]
    public static extern IntPtr GetWindowHandle(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowParentId")]
    public static extern int GetWindowParentId(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowOwnerHandle")]
    public static extern IntPtr GetWindowOwnerHandle(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowParentHandle")]
    public static extern IntPtr GetWindowParentHandle(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowInstance")]
    public static extern IntPtr GetWindowInstance(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowProcessId")]
    public static extern int GetWindowProcessId(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowThreadId")]
    public static extern int GetWindowThreadId(int id);
    [DllImport(name, EntryPoint = "UwcRequestUpdateWindowTitle")]
    public static extern void RequestUpdateWindowTitle(int id);
    [DllImport(name, EntryPoint = "UwcRequestCaptureWindow")]
    public static extern void RequestCaptureWindow(int id, CapturePriority priority);
    [DllImport(name, EntryPoint = "UwcRequestCaptureIcon")]
    public static extern void RequestCaptureIcon(int id);
    [DllImport(name, EntryPoint = "StartCaptureWindow")]
    public static extern void StartCaptureWindow(int id, CapturePriority priority);
    [DllImport(name, EntryPoint = "StopCaptureWindow")]
    public static extern void StopCaptureWindow(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowX")]
    public static extern int GetWindowX(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowY")]
    public static extern int GetWindowY(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowWidth")]
    public static extern int GetWindowWidth(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowHeight")]
    public static extern int GetWindowHeight(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowZOrder")]
    public static extern int GetWindowZOrder(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowBuffer")]
    public static extern IntPtr GetWindowBuffer(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTextureWidth")]
    public static extern int GetWindowTextureWidth(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTextureHeight")]
    public static extern int GetWindowTextureHeight(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTextureOffsetX")]
    public static extern int GetWindowTextureOffsetX(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTextureOffsetY")]
    public static extern int GetWindowTextureOffsetY(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowIconWidth")]
    public static extern int GetWindowIconWidth(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowIconHeight")]
    public static extern int GetWindowIconHeight(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTitleLength")]
    private static extern int GetWindowTitleLength(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTitle", CharSet = CharSet.Unicode)]
    private static extern IntPtr GetWindowTitle_Internal(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowClassNameLength")]
    private static extern int GetWindowClassNameLength(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowClassName", CharSet = CharSet.Ansi)]
    private static extern IntPtr GetWindowClassName_Internal(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowTexturePtr")]
    public static extern IntPtr GetWindowTexturePtr(int id);
    [DllImport(name, EntryPoint = "UwcSetWindowTexturePtr")]
    public static extern void SetWindowTexturePtr(int id, IntPtr texturePtr);
    [DllImport(name, EntryPoint = "UwcGetWindowIconTexturePtr")]
    public static extern IntPtr GetWindowIconTexturePtr(int id);
    [DllImport(name, EntryPoint = "UwcSetWindowIconTexturePtr")]
    public static extern void SetWindowIconTexturePtr(int id, IntPtr texturePtr);
    [DllImport(name, EntryPoint = "UwcGetWindowCaptureMode")]
    public static extern CaptureMode GetWindowCaptureMode(int id);
    [DllImport(name, EntryPoint = "UwcSetWindowCaptureMode")]
    public static extern void SetWindowCaptureMode(int id, CaptureMode mode);
    [DllImport(name, EntryPoint = "UwcGetWindowCursorDraw")]
    public static extern bool GetWindowCursorDraw(int id);
    [DllImport(name, EntryPoint = "UwcSetWindowCursorDraw")]
    public static extern void SetWindowCursorDraw(int id, bool draw);
    [DllImport(name, EntryPoint = "UwcIsWindow")]
    public static extern bool IsWindow(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowVisible")]
    public static extern bool IsWindowVisible(int id);
    [DllImport(name, EntryPoint = "UwcIsAltTabWindow")]
    public static extern bool IsAltTabWindow(int id);
    [DllImport(name, EntryPoint = "UwcIsDesktop")]
    public static extern bool IsDesktop(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowEnabled")]
    public static extern bool IsWindowEnabled(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowUnicode")]
    public static extern bool IsWindowUnicode(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowZoomed")]
    public static extern bool IsWindowZoomed(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowIconic")]
    public static extern bool IsWindowIconic(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowHungUp")]
    public static extern bool IsWindowHungUp(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowTouchable")]
    public static extern bool IsWindowTouchable(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowApplicationFrameWindow")]
    public static extern bool IsApplicationFrameWindow(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowUWP")]
    public static extern bool IsWindowUWP(int id);
    [DllImport(name, EntryPoint = "UwcIsWindowBackground")]
    public static extern bool IsWindowBackground(int id);
    [DllImport(name, EntryPoint = "UwcGetWindowPixel")]
    public static extern Color32 GetWindowPixel(int id, int x, int y);
    [DllImport(name, EntryPoint = "UwcGetWindowPixels")]
    private static extern bool GetWindowPixels_Internal(int id, IntPtr output, int x, int y, int width, int height);
    [DllImport(name, EntryPoint = "UwcRequestCaptureCursor")]
    public static extern void RequestCaptureCursor();
    [DllImport(name, EntryPoint = "UwcGetCursorPosition")]
    public static extern Point GetCursorPosition();
    [DllImport(name, EntryPoint = "UwcGetWindowIdFromPoint")]
    public static extern int GetWindowIdFromPoint(int x, int y);
    [DllImport(name, EntryPoint = "UwcGetWindowIdUnderCursor")]
    public static extern int GetWindowIdUnderCursor();
    [DllImport(name, EntryPoint = "UwcGetCursorX")]
    public static extern int GetCursorX();
    [DllImport(name, EntryPoint = "UwcGetCursorY")]
    public static extern int GetCursorY();
    [DllImport(name, EntryPoint = "UwcGetCursorWidth")]
    public static extern int GetCursorWidth();
    [DllImport(name, EntryPoint = "UwcGetCursorHeight")]
    public static extern int GetCursorHeight();
    [DllImport(name, EntryPoint = "UwcSetCursorTexturePtr")]
    public static extern void SetCursorTexturePtr(IntPtr ptr);
    [DllImport(name, EntryPoint = "UwcGetScreenX")]
    public static extern int GetScreenX();
    [DllImport(name, EntryPoint = "UwcGetScreenY")]
    public static extern int GetScreenY();
    [DllImport(name, EntryPoint = "UwcGetScreenWidth")]
    public static extern int GetScreenWidth();
    [DllImport(name, EntryPoint = "UwcGetScreenHeight")]
    public static extern int GetScreenHeight();
    [DllImport(name, EntryPoint = "UwcIsWindowsGraphicsCaptureSupported")]
    public static extern bool IsWindowsGraphicsCaptureSupported();
    [DllImport(name, EntryPoint = "UwcIsWindowsGraphicsCaptureCursorCaptureEnabledApiSupported")]
    public static extern bool IsWindowsGraphicsCaptureCursorCaptureEnabledApiSupported();

    public static Message[] GetMessages()
    {
        var count = GetMessageCount();
        var messages = new Message[count];

        if (count == 0) return messages;

        var ptr = GetMessages_Internal();
        var size = Marshal.SizeOf(typeof(Message));

        for (int i = 0; i < count; ++i) {
            var data = new IntPtr(ptr.ToInt64() + (size * i));
            messages[i] = (Message)Marshal.PtrToStructure(data, typeof(Message));
        }

        ClearMessages();

        return messages;
    }

    public static string GetWindowTitle(int id)
    {
        var len = GetWindowTitleLength(id);
        var ptr = GetWindowTitle_Internal(id);
        if (ptr != IntPtr.Zero) {
            return Marshal.PtrToStringUni(ptr, len);
        } else {
            return "";
        }
    }

    public static string GetWindowClassName(int id)
    {
        var len = GetWindowClassNameLength(id);
        var ptr = GetWindowClassName_Internal(id);
        if (ptr != IntPtr.Zero) {
            return Marshal.PtrToStringAnsi(ptr, len);
        } else {
            return "";
        }
    }

    public static Color32[] GetWindowPixels(int id, int x, int y, int width, int height)
    {
        var color = new Color32[width * height];       
        GetWindowPixels(id, color, x, y, width, height);
        return color;
    }

    public static bool GetWindowPixels(int id, Color32[] colors, int x, int y, int width, int height)
    {
        if (colors.Length < width * height) {
            Debug.LogErrorFormat("colors is smaller than (width * height).", id, x, y, width, height);
            return false;
        }
		var handle = GCHandle.Alloc(colors, GCHandleType.Pinned);
		var ptr = handle.AddrOfPinnedObject();
        if (!GetWindowPixels_Internal(id, ptr, x, y, width, height)) {
            Debug.LogErrorFormat("GetWindowPixels({0}, {1}, {2}, {3}, {4}) failed.", id, x, y, width, height);
            return false;
        }
        handle.Free();
        return true;
    }
}

}