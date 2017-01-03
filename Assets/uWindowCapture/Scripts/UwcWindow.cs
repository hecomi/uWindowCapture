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
        this.alive = true;
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

    public bool alive
    {
        get;
        set;
    }

    public bool isChild
    {
        get { return owner != System.IntPtr.Zero; }
    }

    public bool visible
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

    public bool enabled
    {
        get { return Lib.IsWindowEnabled(id); }
    }

    public bool unicode
    {
        get { return Lib.IsWindowUnicode(id); }
    }

    public bool zoomed 
    {
        get { return Lib.IsWindowZoomed(id); }
    }

    public bool maximized
    {
        get { return zoomed; }
    }

    public bool iconic
    {
        get { return Lib.IsWindowIconic(id); }
    }

    public bool minimized
    {
        get { return iconic; }
    }

    public bool hungup
    {
        get { return Lib.IsWindowHungUp(id); }
    }

    public bool touchable
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

    public bool shouldBeUpdated
    {
        get;
        set;
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