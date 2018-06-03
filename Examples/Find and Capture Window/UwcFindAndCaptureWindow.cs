using UnityEngine;

namespace uWindowCapture
{

public class UwcFindAndCaptureWindow : MonoBehaviour
{
    UwcWindow window = null;

    [SerializeField] string target = "";
    [SerializeField] CaptureMode mode;
    [SerializeField] Renderer iconRenderer;

    Material iconMaterial_;

    void Start()
    {
        iconMaterial_ = iconRenderer.material;
    }

    void Update()
    {
        if (window == null || !window.isAlive) {
            window = UwcManager.Find(target);
            if (window != null) {
                window.RequestCaptureIcon();
            }
        }

        if (window != null) {
            UpdateScale();
            UpdateWindowTexture();
            UpdateIconTexture();
        }
    }

    void UpdateScale()
    {
        var baseScale = 1000f;
        var width = window.width / baseScale;
        var height = window.height / baseScale;
        transform.localScale = new Vector3(width, height, 1f);
    }

    void UpdateWindowTexture()
    {
        GetComponent<Renderer>().material.mainTexture = window.texture;
        window.captureMode = mode;
        window.RequestCapture(CapturePriority.High);
    }

    void UpdateIconTexture()
    {
        iconMaterial_.mainTexture = window.iconTexture;

        var pos = transform.position;
        var scale = transform.localScale; 
        var iconScale = iconRenderer.transform.localScale;
        scale.z = 0;
        iconRenderer.transform.position = 
            pos + 
            new Vector3((-scale.x + iconScale.x) * 0.5f, (scale.y + iconScale.y) * 0.5f, 0f);
    }
}

}