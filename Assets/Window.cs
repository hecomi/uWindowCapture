using UnityEngine;
using System.Collections;

namespace uWindowCapture
{

public class Window
{
    public int id 
    { 
        get; 
        private set; 
    }

    public Window(System.IntPtr handle)
    {
        this.id = Lib.AddWindow(handle);
        this.handle = handle;
        this.alive = true;
    }

    ~Window()
    {
        Lib.RemoveWindow(id);
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

    public string title
    {
        get { return Lib.GetTitle(id); }
    }

    public int width
    {
        get { return Lib.GetWidth(id); }
    }

    public int height
    {
        get { return Lib.GetHeight(id); }
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

    public void UpdateTextureIfNeeded()
    {
        var w = width;
        var h = height;
        if (!texture || texture.width != w || texture.height != h) {
            if (texture) Object.DestroyImmediate(texture);
            texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            Lib.SetTexturePtr(id, texture.GetNativeTexturePtr());
        }
    }
}

}