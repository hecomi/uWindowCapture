using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTexture))]
public class UwcNewWindowExample : MonoBehaviour
{
    [SerializeField]
    float delay = 1f;
    float delayTimer_ = 0f;
    bool isReady { get { return delayTimer_ > delay; } }

    Renderer renderer_;
    UwcWindowTexture texture_;

    void OnEnable()
    {
        UwcManager.onWindowAdded.AddListener(OnWindowAdded);
        UwcManager.onWindowRemoved.AddListener(OnWindowRemoved);

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
        UwcManager.onWindowAdded.RemoveListener(OnWindowAdded);
        UwcManager.onWindowAdded.RemoveListener(OnWindowRemoved);

        if (texture_) {
            texture_.window = null;
            texture_ = null;
        }
    }

    void Update()
    {
        UpdateRenderer();

        delayTimer_ += Time.deltaTime;
    }

    void UpdateRenderer()
    {
        if (renderer_) {
            renderer_.enabled = texture_.window != null;
        }
    }

    void OnWindowAdded(UwcWindow window)
    {
        if (!isReady) return;

        if (texture_) {
            texture_.window = window;
        }
    }

    void OnWindowRemoved(UwcWindow window)
    {
        if (texture_.window == window) {
            texture_.window = null;
        }
    }
}

}

