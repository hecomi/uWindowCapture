using UnityEngine;

namespace uWindowCapture
{

public class Window
{
    public int id 
    { 
        get; 
        private set; 
    }

    public Window(System.IntPtr handle, int id)
    {
        this.handle = handle;
        this.id = id;
        this.isAlive = true;
    }

    ~Window()
    {
    }

    public System.IntPtr handle
    {
        get;
        private set;
    }

    public System.IntPtr owner
    {
        get { return Lib.GetWindowOwner(id); }
    }

    public bool isAlive
    {
        get;
        set;
    }

    public bool isChild
    {
        get { return owner != System.IntPtr.Zero; }
    }

    public bool isRoot
    {
        get { return owner == System.IntPtr.Zero; }
    }

    public bool isVisible
    {
        get { return Lib.IsWindowVisible(id); }
    }

    public bool isAltTabWindow
    {
        get { return Lib.IsAltTabWindow(id); }
    }

    public bool isDesktop
    {
        get { return Lib.IsDesktop(id); }
    }

    public bool isEnabled
    {
        get { return Lib.IsWindowEnabled(id); }
    }

    public bool isUnicode
    {
        get { return Lib.IsWindowUnicode(id); }
    }

    public bool isZoomed 
    {
        get { return Lib.IsWindowZoomed(id); }
    }

    public bool isMaximized
    {
        get { return isZoomed; }
    }

    public bool isIconic
    {
        get { return Lib.IsWindowIconic(id); }
    }

    public bool isMinimized
    {
        get { return isIconic; }
    }

    public bool isHungup
    {
        get { return Lib.IsWindowHungUp(id); }
    }

    public bool isTouchable
    {
        get { return Lib.IsWindowTouchable(id); }
    }

    public string title
    {
        get { return Lib.GetWindowTitle(id); } 
    }

    public int x
    {
        get { return Lib.GetWindowX(id); }
    }

    public int y
    {
        get { return Lib.GetWindowY(id); }
    }

    public int width
    {
        get { return Lib.GetWindowWidth(id); }
    }

    public int height
    {
        get { return Lib.GetWindowHeight(id); }
    }

    public int zOrder
    {
        get { return Lib.GetWindowZOrder(id); }
    }

    public Texture2D texture
    {
        get; 
        private set;
    }

    public CaptureMode captureMode
    {
        get { return Lib.GetWindowCaptureMode(id); }
        set { Lib.SetWindowCaptureMode(id, value); }
    }

    public delegate void Event();

    public Event onCaptured
    {
        get;
        set;
    }

    public Event onSizeChanged
    {
        get;
        set;
    }

    public void StartCapture()
    {
        Lib.StartCaptureWindow(id);
    }

    public void StopCapture()
    {
        Lib.StopCaptureWindow(id);
    }

    public void RequestCapture(CapturePriority priority = CapturePriority.Immediate)
    {
        UpdateTextureIfNeeded();
        Lib.RequestCaptureWindow(id, priority);
    }

    public void UpdateTextureIfNeeded()
    {
        var w = width;
        var h = height;
        if (w == 0 || h == 0) return;
        if (!texture || texture.width != w || texture.height != h) {
            if (texture) Object.DestroyImmediate(texture);
            texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            Lib.SetWindowTexturePtr(id, texture.GetNativeTexturePtr());
        }
    }
}

}