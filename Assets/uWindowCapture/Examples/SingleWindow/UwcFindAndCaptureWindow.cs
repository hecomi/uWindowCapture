using UnityEngine;
using uWindowCapture;

namespace uWindowCapture
{

public class UwcFindAndCaptureWindow : MonoBehaviour
{
    Window window = null;

    public string target = "";
    public CaptureMode mode;
    public Renderer iconRenderer;

    Material iconMaterial_;

    void Update()
    {
        if (window == null || !window.isAlive) {
            window = UwcManager.Find(target);
            if (window != null) {
                Debug.Log(window);
            }
        }

        if (window != null) {
            UpdateScale();
            UpdateTexture();
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

    void UpdateTexture()
    {
        GetComponent<Renderer>().material.mainTexture = window.texture;
        window.captureMode = mode;
        window.RequestCapture(CapturePriority.High);
    }

    void UpdateIconTexture()
    {
        if (!iconRenderer) return;
        if (!iconMaterial_) {
            iconMaterial_ = iconRenderer.material;
        }
        iconMaterial_.mainTexture = window.iconTexture;

        {
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

}