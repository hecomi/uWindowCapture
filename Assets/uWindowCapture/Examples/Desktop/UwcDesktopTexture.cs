using UnityEngine;

namespace uWindowCapture
{

public class UwcDesktopTexture : MonoBehaviour 
{
    Renderer renderer_;
    Material material_;

    [SerializeField, Tooltip("Window scale (meter per 1000 pixel)")]
    float baseScale = 1f;

    [SerializeField]
    int desktopIndex = 0;

    [SerializeField] 
    CapturePriority priority = CapturePriority.High;

    void Start()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material;
    }

    void Update()
    {
        var window = UwcManager.FindDesktop(desktopIndex);
        UpdateRenderer(window);
        UpdateScale(window);
    }

    void UpdateRenderer(UwcWindow window)
    { 
        if (window == null) {
            material_.mainTexture = null; 
            renderer_.enabled = false;
        } else {
            window.RequestCapture(priority);
            material_.mainTexture = window.texture;
            renderer_.enabled = true;
        }
    }

    void UpdateScale(UwcWindow window)
    {
        if (window == null) return;

        var scale = baseScale / 1000f;
        var width = window.width * scale;
        var height = window.height * scale;
        transform.localScale = new Vector3(width, height, 1f);
    }
}

}