using UnityEngine;

namespace uWindowCapture
{

public class UwcWindowObject : MonoBehaviour
{
    public Window window { get; set; }
    public CaptureMode mode = CaptureMode.PrintWindow;
    Material material_;

    void Start()
    {
        mode = window.captureMode;
        material_ = GetComponent<Renderer>().material; // clone
    }

    void Update()
    {
        if (material_.mainTexture != window.texture) {
            material_.mainTexture = window.texture;
        }
        window.captureMode = mode;
    }

    void OnWillRenderObject()
    {
        window.shouldBeUpdated = (Random.Range(0, 10) == 0);
    }
}

}