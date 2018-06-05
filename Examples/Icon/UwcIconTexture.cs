using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(Renderer))]
public class UwcIconTexture : MonoBehaviour
{
    [SerializeField] UwcWindowObject windowObject;
    [SerializeField] bool followWindow = true;

    Renderer renderer_;
    Material material_;

    void Start()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material;
    }

    void Update()
    {
        if (windowObject != null && windowObject.window != null) {
            UpdateIconTexture();
            UpdateTransform();
        }
    }

    void UpdateIconTexture()
    {
        windowObject.window.RequestCaptureIcon();
        material_.mainTexture = windowObject.window.iconTexture;
    }

    void UpdateTransform()
    {
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