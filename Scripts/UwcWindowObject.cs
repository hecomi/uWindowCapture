using UnityEngine;

namespace uWindowCapture
{

public class UwcWindowObject : MonoBehaviour
{
    public Window window { get; set; }

    public CaptureMode captureMode = CaptureMode.PrintWindow;
    public int skipFrame = 10;

    int updatedFrame_ = 0;
    Material material_;

    void Awake()
    {
        material_ = GetComponent<Renderer>().material; // clone
    }

    void Start()
    {
        //captureMode = window.captureMode;
        window.onSizeChanged += OnSizeChanged;
        window.onCaptured += OnCaptured;
        window.RequestCapture(CapturePriority.High);
    }

    void OnDestroy()
    {
        window.onCaptured -= OnCaptured;
        window.onSizeChanged -= OnSizeChanged;
    }

    void Update()
    {
        if (material_.mainTexture != window.texture) {
            material_.mainTexture = window.texture;
        }

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

    void OnCaptured()
    {
        window.onCaptured -= OnCaptured;
    }

    void OnSizeChanged()
    {
        window.RequestCapture();
    }
}

}