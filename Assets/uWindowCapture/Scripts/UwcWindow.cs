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
        this.onCaptured += OnCaptured;
        this.onSizeChanged += OnSizeChanged;
    }

    ~Window()
    {
        this.onCaptured -= OnCaptured;
        this.onSizeChanged -= OnSizeChanged;
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

    public int processId
    {
        get { return Lib.GetWindowProcessId(id); }
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

    public int bufferWidth
    {
        get { return Lib.GetWindowBufferWidth(id); }
    }

    public int bufferHeight
    {
        get { return Lib.GetWindowBufferHeight(id); }
    }

    public Texture2D texture
    {
        get;
        private set;
    }

    private Texture2D backTexture_;
    private bool willTextureSizeChange_ = false;

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

    public void RequestCapture(CapturePriority priority = CapturePriority.High)
    {
        Lib.RequestCaptureWindow(id, priority);
    }

    void OnSizeChanged()
    {
        var w = bufferWidth;
        var h = bufferHeight;
        if (w == 0 || h == 0) return;

        if (!texture || texture.width != w || texture.height != h) {
            backTexture_ = new Texture2D(w, h, TextureFormat.BGRA32, false);
            Lib.SetWindowTexturePtr(id, backTexture_.GetNativeTexturePtr());
            willTextureSizeChange_ = true;
        }
    }

    void OnCaptured()
    {
        if (willTextureSizeChange_) {
            Object.DestroyImmediate(texture);
            texture = backTexture_;
            backTexture_ = null;
            willTextureSizeChange_ = false;
        }
    }
}

}