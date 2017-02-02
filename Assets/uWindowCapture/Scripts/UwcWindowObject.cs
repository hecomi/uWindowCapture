using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowObject : MonoBehaviour
{
    private static HashSet<UwcWindowObject> list_ = new HashSet<UwcWindowObject>();
    public static HashSet<UwcWindowObject> list
    {
        get { return list_; }
    }

    private UwcWindow window_;
    public UwcWindow window 
    { 
        get 
        {
            return window_;
        }
        set 
        {
            if (window_ != null) {
                window_.onCaptured.RemoveListener(OnCaptured);
            }

            onWindowChanged_.Invoke(value, window_);
            window_ = value;

            if (window_ != null) {
                captureMode = window_.captureMode;
                window_.RequestCapture(CapturePriority.High);
                window_.onCaptured.AddListener(OnCaptured);
            }

            renderer_.enabled = false;
        }
    }

    public UwcWindowObjectManager manager { get; set; }
    public UwcWindowObject parent { get; set; }

    UwcWindowChangeEvent onWindowChanged_ = new UwcWindowChangeEvent();
    public UwcWindowChangeEvent onWindowChanged
    {
        get { return onWindowChanged_; }
    }

    [Tooltip("Window scale (meter per 1000 pixel)")]
    public float scale = 1f;
    public float basePixel
    {
        get { return 1000f / scale; }
    }

    public float width
    {
        get 
        {
            var meshWidth = meshFilter_.sharedMesh.bounds.extents.x * 2f;
            var baseWidth = meshWidth * basePixel;
            return window.width / baseWidth;
        }
    }

    public float height
    {
        get 
        {
            var meshHeight = meshFilter_.sharedMesh.bounds.extents.y * 2f;
            var baseHeight = meshHeight * basePixel;
            return window.height / baseHeight;
        }
    }

    [Tooltip("CaptureMethod" +
        "- BitBlt: fast but cannot capture some windows.\n" +
        "- BitBltAlpha: BitBlt with alpha.\n" +
        "- PrintWindow: slow but can capture almost all windows.")]
    public CaptureMode captureMode = CaptureMode.PrintWindow;

    public int skipFrame = 10;
    int updatedFrame_ = 0;
    bool hasBeenCaptured_ = false;

    Material material_;
    Renderer renderer_;
    MeshFilter meshFilter_;

    void Awake()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material; // clone
        meshFilter_ = GetComponent<MeshFilter>();

        list_.Add(this);
    }

    void OnDestroy()
    {
        list_.Remove(this);
    }

    void Update()
    {
        if (window == null) return;

        UpdateTexture();
        UpdateRenderer();

        updatedFrame_++;
    }

    void UpdateTexture()
    {
        if (window == null) return;

        if (material_.mainTexture != window.texture) {
            material_.mainTexture = window.texture;
        }
    }

    void UpdateRenderer()
    {
        if (hasBeenCaptured_) {
            renderer_.enabled = !window.isIconic && window.isVisible;
        }
    }

    void OnWillRenderObject()
    {
        if (window == null) return;

        window.captureMode = captureMode;

        if (updatedFrame_ % skipFrame == 0) {
            var priority = CapturePriority.Low;
            if (window == UwcManager.cursorWindow) {
                priority = CapturePriority.High;
            } else if (window.zOrder < UwcSetting.MiddlePriorityMaxZ) {
                priority = CapturePriority.Middle;
            }
            window.RequestCapture(priority);
        }
    }

    void OnCaptured()
    {
        hasBeenCaptured_ = true;

        if (window.isAltTabWindow) {
            name = window.title;
        }
    }
}

}