using UnityEngine;

namespace uWindowCapture
{

public class UwcWindowObject : MonoBehaviour
{
    public Window window { get; set; }
    public bool isChild { get; /* only window manager set this. */ set; }

    public CaptureMode captureMode = CaptureMode.PrintWindow;
    public int skipFrame = 10;
    public bool updateTitleEveryFrame = false;

    int updatedFrame_ = 0;
    Material material_;

    void Awake()
    {
        material_ = GetComponent<Renderer>().material; // clone
    }

    void Start()
    {
        captureMode = window.captureMode;
        window.RequestCapture(CapturePriority.High);
    }

    void Update()
    {
        if (material_.mainTexture != window.texture) {
            material_.mainTexture = window.texture;
        }

        if (updateTitleEveryFrame) {
            gameObject.name = window.title;
        }
        gameObject.name = ((window.handle == Lib.GetForegroundWindow()) ? "★" : "") + " " + window.processId + " " + window.threadId + " " + window.title;

        updatedFrame_++;
    }

    void OnWillRenderObject()
    {
        window.captureMode = captureMode;

        if (updatedFrame_ % skipFrame == 0) {
            var priority = (window.handle == Lib.GetForegroundWindow()) ?
                CapturePriority.High : 
                CapturePriority.Low;
            window.RequestCapture(priority);
        }
    }
}

}