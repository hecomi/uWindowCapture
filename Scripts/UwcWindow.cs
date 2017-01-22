using UnityEngine;
using UnityEngine.Events;

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

        onCaptured.AddListener(OnCaptured);
        onSizeChanged.AddListener(OnSizeChanged);
        onIconCaptured.AddListener(OnIconCaptured);

        CreateIconTexture();
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

    public System.IntPtr parent
    {
        get { return Lib.GetWindowParent(id); }
    }

    public System.IntPtr instance
    {
        get { return Lib.GetWindowInstance(id); }
    }

    public int processId
    {
        get { return Lib.GetWindowProcessId(id); }
    }

    public int threadId
    {
        get { return Lib.GetWindowThreadId(id); }
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
        set { Move(value, y); }
    }

    public int y
    {
        get { return Lib.GetWindowY(id); }
        set { Move(x, value); }
    }

    public int width
    {
        get { return Lib.GetWindowWidth(id); }
        set { Scale(value, height); }
    }

    public int height
    {
        get { return Lib.GetWindowHeight(id); }
        set { Scale(width, value); }
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

    public int iconWidth
    {
        get { return Lib.GetWindowIconWidth(id); }
    }

    public int iconHeight
    {
        get { return Lib.GetWindowIconHeight(id); }
    }

    private Texture2D backTexture_;
    private bool willTextureSizeChange_ = false;
    public Texture2D texture
    {
        get;
        private set;
    }

    private Texture2D iconTexture_;
    private Texture2D errorIconTexture_;
    private bool hasIconTextureCaptured_ = false;
    public bool hasIconTexture
    {
        get { return hasIconTextureCaptured_; }
    }

    public Texture2D iconTexture
    {
        get { return hasIconTextureCaptured_ ? iconTexture_ : errorIconTexture_; }
    }

    public CaptureMode captureMode
    {
        get { return Lib.GetWindowCaptureMode(id); }
        set { Lib.SetWindowCaptureMode(id, value); }
    }

    private UnityEvent onCaptured_ = new UnityEvent();
    public UnityEvent onCaptured 
    { 
        get { return onCaptured_; } 
    }

    private UnityEvent onSizeChanged_ = new UnityEvent();
    public UnityEvent onSizeChanged
    {
        get { return onSizeChanged_; } 
    }

    private UnityEvent onIconCaptured_ = new UnityEvent();
    public UnityEvent onIconCaptured 
    { 
        get { return onIconCaptured_; } 
    }

    public void RequestCapture(CapturePriority priority = CapturePriority.High)
    {
        Lib.RequestCaptureWindow(id, priority);
    }

    void OnSizeChanged()
    {
        CreateWindowTexture();
    }

    void OnCaptured()
    {
        UpdateWindowTexture();
    }

    void OnIconCaptured()
    {
        hasIconTextureCaptured_ = true;
    }

    void CreateWindowTexture()
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

    void UpdateWindowTexture()
    {
        if (willTextureSizeChange_) {
            Object.DestroyImmediate(texture);
            texture = backTexture_;
            backTexture_ = null;
            willTextureSizeChange_ = false;
        }
    }

    void CreateIconTexture()
    {
        var w = iconWidth;
        var h = iconHeight;
        if (w == 0 || h == 0) return;
        iconTexture_ = new Texture2D(w, h, TextureFormat.BGRA32, false);
        Lib.SetWindowIconTexturePtr(id, iconTexture_.GetNativeTexturePtr());
        errorIconTexture_ = Resources.Load<Texture2D>("uWindowCapture/Textures/uWC_No_Image");
    }

    public void Move(int x, int y)
    {
        if (!Lib.MoveWindow(id, x, y)) {
            Debug.Log("MoveWindow() failed.");
        }
    }

    public void Scale(int width, int height)
    {
        if (!Lib.ScaleWindow(id, width, height)) {
            Debug.Log("ScaleWindow() failed.");
        }
    }

    public void MoveAndScale(int x, int y, int width, int height)
    {
        if (!Lib.MoveAndScaleWindow(id, x, y, width, height)) {
            Debug.Log("MOveAndScaleWindow() failed.");
        }
    }
}

}