using UnityEngine;

namespace uWindowCapture
{

public class UwcGetPixelsExample : MonoBehaviour
{
    [SerializeField] UwcWindowTexture uwcTexture;

    [SerializeField] int x = 100;
    [SerializeField] int y = 100;
    [SerializeField] int w = 64;
    [SerializeField] int h = 32;
    
    public Texture2D texture;
    Color32[] colors;

    void CreateTextureIfNeeded()
    {
        if (!texture || texture.width != w || texture.height != h)
        {
            colors = new Color32[w * h];
            texture = new Texture2D(w, h, TextureFormat.RGBA32, false);
            GetComponent<Renderer>().material.mainTexture = texture;
        }
    }

    void Start()
    {
        CreateTextureIfNeeded();
    }

    void Update()
    {
        CreateTextureIfNeeded();

        var window = uwcTexture.window;
        if (window == null || window.width == 0) return;

        // GetPixels() can be run in another thread
        if (window.GetPixels(colors, x, y, w, h)) {
            texture.SetPixels32(colors);
            texture.Apply();
        }
    }
}

}