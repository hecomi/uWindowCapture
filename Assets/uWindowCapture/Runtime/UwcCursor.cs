using UnityEngine;

namespace uWindowCapture
{

public class UwcCursor
{
    public UwcCursor()
    {
        onCaptured.AddListener(OnCaptured);
    }

    public int x
    {
        get { return Lib.GetCursorX(); }
    }

    public int y
    {
        get { return Lib.GetCursorY(); }
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

    public void RequestCapture()
    {
        Lib.RequestCaptureCursor();
    }

    void OnCaptured()
    {
    }

    public void CreateTextureIfNeeded()
    {
        var w = width;
        var h = height;
        if (w == 0 || h == 0) return;

        if (!texture || texture.width != w || texture.height != h) {
            texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            texture.filterMode = FilterMode.Point;
            texture.wrapMode = TextureWrapMode.Clamp;
            Lib.SetCursorTexturePtr(texture.GetNativeTexturePtr());
            onTextureChanged.Invoke();
        }
    }
}

}