using UnityEngine;
using UnityEngine.Events;

namespace uWindowCapture
{

public class UwcWindow
{
    public UwcWindow(int id)
    {
        this.id = id;
        isAlive = true;

        onCaptured.AddListener(OnCaptured);
        onSizeChanged.AddListener(OnSizeChanged);
        onIconCaptured.AddListener(OnIconCaptured);

        CreateIconTexture();

        parentWindow = UwcManager.FindParent(id);
        if (parentWindow != null) {
            parentWindow.onChildAdded.Invoke(this);
        }
    }

    public int id 
    { 
        get; 
        private set; 
    }

    public UwcWindow parentWindow
    {
        get;
        private set;
    }

    public System.IntPtr handle
    {
        get { return Lib.GetWindowHandle(id); }
    }

    public System.IntPtr ownerHandle
    {
        get { return Lib.GetWindowOwnerHandle(id); }
    }

    public System.IntPtr parentHandle
    {
        get { return Lib.GetWindowParentHandle(id); }
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

    public bool isValid
    {
        get { return Lib.CheckWindowExistence(id); }
    }

    public bool isAlive
    {
        get;
        set;
    }

    public bool isRoot
    {
        get { return parentWindow == null; }
    }

    public bool isChild
    {
        get { return !isRoot; }
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

    public bool isApplicationFrameWindow
    {
        get { return Lib.IsApplicationFrameWindow(id); }
    }

    public bool isUWP
    {
        get { return Lib.IsWindowUWP(id); }
    }

    public bool isBackground
    {
        get { return Lib.IsWindowBackground(id); }
    }

    public string title
    {
        get { return Lib.GetWindowTitle(id); } 
    }

    public string className
    {
        get { return Lib.GetWindowClassName(id); } 
    }

    public int rawX
    {
        get { return Lib.GetWindowX(id); }
    }

    public int rawY
    {
        get { return Lib.GetWindowY(id); }
    }

    public int rawWidth
    {
        get { return Lib.GetWindowWidth(id); }
    }

    public int rawHeight
    {
        get { return Lib.GetWindowHeight(id); }
    }

    public int x
    {
        get { return rawX + Lib.GetWindowTextureOffsetX(id); }
    }

    public int y
    {
        get { return rawY + Lib.GetWindowTextureOffsetY(id); }
    }

    public int width
    {
        get { return Lib.GetWindowTextureWidth(id); }
    }

    public int height
    {
        get { return Lib.GetWindowTextureHeight(id); }
    }

    public int zOrder
    {
        get { return Lib.GetWindowZOrder(id); }
    }

    public System.IntPtr buffer
    {
        get { return Lib.GetWindowBuffer(id); }
    }

    public int textureOffsetX
    {
        get { return Lib.GetWindowTextureOffsetX(id); }
    }

    public int textureOffsetY
    {
        get { return Lib.GetWindowTextureOffsetY(id); }
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

    public bool cursorDraw
    {
        get { return Lib.GetWindowCursorDraw(id); }
        set { Lib.SetWindowCursorDraw(id, value); }
    }

    private UnityEvent onCaptured_ = new UnityEvent();
    public UnityEvent onCaptured 
    { 
        get { return onCaptured_; } 
    }

    private bool isFirstSizeChangedEvent_ = true;
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

    public class ChildAddedEvent : UnityEvent<UwcWindow> {}
    private ChildAddedEvent onChildAdded_ = new ChildAddedEvent();
    public ChildAddedEvent onChildAdded
    { 
        get { return onChildAdded_; } 
    }

    public class ChildRemovedEvent : UnityEvent<UwcWindow> {}
    private ChildRemovedEvent onChildRemoved_ = new ChildRemovedEvent();
    public ChildRemovedEvent onChildRemoved
    { 
        get { return onChildRemoved_; } 
    }

    public void RequestUpdateTitle()
    {
        Lib.RequestUpdateWindowTitle(id);
    }

    public void RequestCaptureIcon()
    {
        Lib.RequestCaptureIcon(id);
    }

    public void RequestCapture(CapturePriority priority = CapturePriority.High)
    {
        if (!texture) {
            CreateWindowTexture();
        }
        Lib.RequestCaptureWindow(id, priority);
    }

    void OnSizeChanged()
    {
        if (isFirstSizeChangedEvent_) {
            isFirstSizeChangedEvent_ = false;
            return;
        }

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

    void CreateWindowTexture(bool force = false)
    {
        var w = width;
        var h = height;
        if (w <= 0 || h <= 0) return;

        if (force || !texture || texture.width != w || texture.height != h) {
            if (backTexture_) {
                Object.DestroyImmediate(backTexture_);
            }
            try {
                backTexture_ = new Texture2D(w, h, TextureFormat.BGRA32, false);
                Lib.SetWindowTexturePtr(id, backTexture_.GetNativeTexturePtr());
                willTextureSizeChange_ = true;
            } catch (System.Exception e) {
                Debug.LogError(e.Message);
                Debug.LogErrorFormat("Width: {0}, Height: {1}", w, h);
            }
        }
    }

    void UpdateWindowTexture()
    {
        if (willTextureSizeChange_) {
            if (texture) {
                Object.DestroyImmediate(texture);
            }
            texture = backTexture_;
            backTexture_ = null;
            willTextureSizeChange_ = false;
        }
    }

    public void ResetWindowTexture()
    {
        CreateWindowTexture(true);
    }

    void CreateIconTexture()
    {
        var w = iconWidth;
        var h = iconHeight;
        if (w == 0 || h == 0) return;
        iconTexture_ = new Texture2D(w, h, TextureFormat.BGRA32, false);
        iconTexture_.filterMode = FilterMode.Bilinear;
        iconTexture_.wrapMode = TextureWrapMode.Clamp;
        Lib.SetWindowIconTexturePtr(id, iconTexture_.GetNativeTexturePtr());
        errorIconTexture_ = Resources.Load<Texture2D>("uWindowCapture/Textures/uWC_No_Image");
    }

    public Color32[] GetPixels(int x, int y, int width, int height)
    {
        return Lib.GetWindowPixels(id, x, y, width, height);
    }

    public bool GetPixels(Color32[] colors, int x, int y, int width, int height)
    {
        return Lib.GetWindowPixels(id, colors, x, y, width, height);
    }

    public Color32 GetPixel(int x, int y)
    {
        return Lib.GetWindowPixel(id, x, y);
    }
}

}