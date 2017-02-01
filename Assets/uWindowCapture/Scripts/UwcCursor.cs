using UnityEngine;

namespace uWindowCapture
{

public class UwcCursor
{
    public UwcCursor()
    {
        onCaptured.AddListener(OnCaptured);
    }

    public Point point 
    {
        get { return Lib.GetCursorPosition(); }
    }

    public int x
    {
        get { return point.x; }
    }

    public int y
    {
        get { return point.y; }
    }

    public int width
    {
        get { return Lib.GetCursorWidth(); }
    }

    public int height
    {
        get { return Lib.GetCursorHeight(); }
    }

    public Texture2D texture
    {
        get;
        private set;
    }

    UwcEvent onCaptured_ = new UwcEvent();
    public UwcEvent onCaptured
    {
        get { return onCaptured_; }
    }

    UwcEvent onTextureChanged_ = new UwcEvent();
    public UwcEvent onTextureChanged
    {
        get { return onTextureChanged_; }
    }

    void OnCaptured()
    {
    }

    public void CreateTexture()
    {
        var w = width;
        var h = height;
        if (w == 0 || h == 0) return;

        if (!texture || texture.width != w || texture.height != h) {
            texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            Lib.SetCursorTexturePtr(texture.GetNativeTexturePtr());
            onTextureChanged.Invoke();
        }
    }
}

}