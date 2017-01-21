using UnityEngine;
using uWindowCapture;

namespace uWindowCapture
{

public class UwcFindAndCaptureWindow : MonoBehaviour
{
    Window window = null;
    public string target = "";
    public CaptureMode mode;

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
}

}