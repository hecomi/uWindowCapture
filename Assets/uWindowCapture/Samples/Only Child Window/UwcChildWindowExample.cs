using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTexture))]
public class UwcChildWindowExample : MonoBehaviour
{
    [SerializeField]
    string partialWindowName = "Unity";

    Renderer renderer_;
    UwcWindowTexture texture_;
    UwcWindow window_;

    void OnEnable()
    {
        texture_ = GetComponent<UwcWindowTexture>();
        texture_.window = null;
        texture_.searchTiming = WindowSearchTiming.Manual;
        texture_.updateTitle = false;
        texture_.createChildWindows = false;
        texture_.updateScaleForcely = true;

        renderer_ = GetComponent<Renderer>();
    }

    void OnDisable()
    {
        if (window_ != null) {
            window_.onChildAdded.RemoveListener(OnChildAdded);
            window_.onChildRemoved.RemoveListener(OnChildRemoved);
            window_ = null;
        }

        if (texture_) {
            texture_.window = null;
            texture_ = null;
        }
    }

    void Update()
    {
        UpdateRenderer();
        UpdateWindow();
    }

    void UpdateRenderer()
    {
        if (renderer_) {
            renderer_.enabled = texture_.window != null;
        }
    }

    void UpdateWindow()
    {
        if (window_ != null) return;

        var window = UwcManager.Find(partialWindowName, false);
        if (window == null) return;

        window_ = window;
        window_.onChildAdded.AddListener(OnChildAdded);
        window_.onChildRemoved.AddListener(OnChildRemoved);
    }

    void OnChildAdded(UwcWindow childWindow)
    {
        if (texture_) {
            texture_.window = childWindow;
        }
    }

    void OnChildRemoved(UwcWindow childWindow)
    {
        if (texture_.window == childWindow) {
            texture_.window = null;
        }
    }
}

}
