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

    public bool alive
    {
        get;
        set;
    }

    public bool visible
    {
        get { return Lib.IsWindowVisible(id); }
    }

    public string title
    {
        get { return Lib.GetWindowTitle(id); } 
    }

    public int width
    {
        get { return Lib.GetWindowWidth(id); }
    }

    public int height
    {
        get { return Lib.GetWindowHeight(id); }
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
        set { Lib.SetWindowCaptureMode(id, value); }
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