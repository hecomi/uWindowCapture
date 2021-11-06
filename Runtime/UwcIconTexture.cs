using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(Renderer))]
public class UwcIconTexture : MonoBehaviour
{
    [SerializeField] UwcWindowTexture windowTexture_;
    public UwcWindowTexture windowTexture
    {
        get
        {
            return windowTexture_;
        }
        set
        {
            windowTexture_ = value;
            if (windowTexture_) {
                window = windowTexture_.window;
            }
        }
    }

    UwcWindow window_ = null;
    public UwcWindow window
    {
        get
        {
            return window_;
        }
        set
        {
            window_ = value;

            if (window_ != null) {
                if (!window_.hasIconTexture) {
                    window_.onIconCaptured.AddListener(OnIconCaptured);
                    window_.RequestCaptureIcon();
                } else {
                    OnIconCaptured();
                }
            }
        }
    }

    bool isValid
    {
        get
        {
            return window != null;
        }
    }

    void Update()
    {
        if (windowTexture != null) {
            if (window == null || window != windowTexture_.window) {
                window = windowTexture_.window;
            }
        }
    }

    void OnIconCaptured()
    {
        if (!isValid) return;

        var renderer = GetComponent<Renderer>();
        renderer.material.mainTexture = window.iconTexture;
        window.onIconCaptured.RemoveListener(OnIconCaptured);
    }
}

}