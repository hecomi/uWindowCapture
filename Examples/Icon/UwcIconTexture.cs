using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(Renderer))]
public class UwcIconTexture : MonoBehaviour
{
    [SerializeField] UwcWindowObject windowObject;

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
        if (windowObject != null && windowObject.window != null) {
            windowObject.window.RequestCaptureIcon();
            material_.mainTexture = windowObject.window.iconTexture;
            renderer_.enabled = true;
        } else {
            material_.mainTexture = null;
            renderer_.enabled = false;
        }
    }

    void UpdateTransform()
    {
        if (windowObject == null) return;

        var windowPos = windowObject.transform.position;
        var windowScale = windowObject.transform.localScale; 
        var iconScale = transform.localScale;
        windowScale.z = 0;
        transform.position = 
            windowPos + 
            new Vector3((-windowScale.x + iconScale.x) * 0.5f, (windowScale.y + iconScale.y) * 0.5f, 0f);
    }
}

}