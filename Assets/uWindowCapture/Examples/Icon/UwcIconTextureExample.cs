using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(Renderer))]
public class UwcIconTextureExample : MonoBehaviour
{
    [SerializeField] UwcWindowTexture windowTexture;

    Renderer renderer_;
    Material material_;
    Vector3 scale_;

    void Start()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material;
    }

    void Update()
    {
        UpdateIconTexture();
        UpdateTransform();
    }

    void UpdateIconTexture()
    {
        if (windowTexture != null && windowTexture.window != null) {
            windowTexture.window.RequestCaptureIcon();
            material_.mainTexture = windowTexture.window.iconTexture;
            renderer_.enabled = true;
        } else {
            material_.mainTexture = null;
            renderer_.enabled = false;
        }
    }

    void UpdateTransform()
    {
        if (windowTexture == null) return;

        var windowPos = windowTexture.transform.position;
        var windowScale = windowTexture.transform.localScale; 
        var iconScale = transform.localScale;
        windowScale.z = 0;
        transform.position = 
            windowPos + 
            new Vector3((-windowScale.x + iconScale.x) * 0.5f, (windowScale.y + iconScale.y) * 0.5f, 0f);
    }
}

}