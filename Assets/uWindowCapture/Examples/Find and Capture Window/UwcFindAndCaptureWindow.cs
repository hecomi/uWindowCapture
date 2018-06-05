using UnityEngine;

namespace uWindowCapture
{

public class UwcFindAndCaptureWindow : MonoBehaviour
{
    UwcWindow window_ = null;
    string target_;

    [SerializeField] string target;
    [SerializeField] CaptureMode mode = CaptureMode.PrintWindow;
    [SerializeField] CapturePriority priority = CapturePriority.High;

    void Update()
    {
        if (target_ != target) {
            window_ = null;
            target_ = target;
        }

        if (window_ == null || !window_.isAlive) {
            window_ = UwcManager.Find(target);
            if (window_ != null) {
                window_.RequestCaptureIcon();
            }
        }

        if (window_ != null) {
            UpdateScale();
            UpdateWindowTexture();
        }
    }

    void UpdateScale()
    {
        var baseScale = 1000f;
        var width = window_.width / baseScale;
        var height = window_.height / baseScale;
        transform.localScale = new Vector3(width, height, 1f);
    }

    void UpdateWindowTexture()
    {
        GetComponent<Renderer>().material.mainTexture = window_.texture;
        window_.captureMode = mode;
        window_.RequestCapture(CapturePriority.High);
    }
}

}