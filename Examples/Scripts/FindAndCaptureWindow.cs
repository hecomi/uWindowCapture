using UnityEngine;
using uWindowCapture;

public class FindAndCaptureWindow : MonoBehaviour
{
    Window window = null;
    public string target = "";
    public CaptureMode mode;

    void OnDestroy()
    {
        if (window != null) {
            window.StopCapture();
        }
    }

    void Update()
    {
        if (window == null || !window.isAlive) {
            window = UwcManager.Find(target);
            if (window != null) {
                window.StartCapture();
                Debug.Log(window);
            }
        }

        if (window != null) {
            GetComponent<Renderer>().material.mainTexture = window.texture;
            window.captureMode = mode;
            window.RequestCapture();
        }
    }
}
