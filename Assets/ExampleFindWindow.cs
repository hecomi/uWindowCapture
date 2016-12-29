using UnityEngine;

public class ExampleFindWindow : MonoBehaviour
{
    uWindowCapture.Window window = null;
    public string target = "";
    public uWindowCapture.CaptureMode mode;

    void Update()
    {
        if (window == null || !window.alive) {
            window = uWindowCapture.Manager.Find(target);
            if (window != null) {
                Debug.Log(window.title);
            }
        }

        if (window != null) {
            GetComponent<Renderer>().material.mainTexture = window.texture;
            window.shouldBeUpdated = true;
            window.captureMode = mode;
        }
    }
}
