using UnityEngine;
using uWindowCapture;

public class FindWindow : MonoBehaviour
{
    Window window = null;
    public string target = "";
    public CaptureMode mode;

    void Update()
    {
        if (window == null || !window.isAlive) {
            window = UwcManager.Find(target);
        }

        if (window != null) {
            GetComponent<Renderer>().material.mainTexture = window.texture;
            window.shouldBeUpdated = true;
            window.captureMode = mode;
        }
    }
}
