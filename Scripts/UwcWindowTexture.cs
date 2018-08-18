using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public enum CaptureRequestTiming
{
    EveryFrame = 0,
    OnlyWhenVisible = 1,
    Manual = 2,
}

public enum ScaleControlMode
{
    BaseScale = 0,
    FixedWidth = 1,
    FixedHeight = 2,
    Manual = 3,
}

public class UwcWindowTexture : MonoBehaviour
{
    public CaptureMode captureMode = CaptureMode.PrintWindow;
    public CapturePriority capturePriority = CapturePriority.Auto;
    public CaptureRequestTiming captureRequestTiming = CaptureRequestTiming.OnlyWhenVisible;
    public int captureFrameRate = 30;
    public bool cursorDraw = true;

    float captureTimer_ = 0f;
    bool hasBeenCaptured_ = false;

    public string partialWindowTitle 
    {
        get 
        {
            return partialWindowTitle_;
        }
        set 
        {
            isPartialWindowTitleChanged_ = true;
            partialWindowTitle_ = value;
        }
    }

    [SerializeField]
    string partialWindowTitle_;
    bool isPartialWindowTitleChanged_ = false;

    public ScaleControlMode scaleControlMode = ScaleControlMode.BaseScale;
    public float scalePer1000Pixel = 1f;

    private static HashSet<UwcWindowTexture> list_ = new HashSet<UwcWindowTexture>();
    public static HashSet<UwcWindowTexture> list
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
                window_.onCaptured.AddListener(OnCaptured);
                window_.RequestCapture(CapturePriority.High);
            }
        }
    }

    public UwcWindowTextureManager manager { get; set; }
    public UwcWindowTexture parent { get; set; }

    UwcWindowChangeEvent onWindowChanged_ = new UwcWindowChangeEvent();
    public UwcWindowChangeEvent onWindowChanged
    {
        get { return onWindowChanged_; }
    }

    public float basePixel
    {
        get { return 1000f / scalePer1000Pixel; }
    }

    public float width 
    {
        get 
        {
            if (window == null) return 0f;

            switch (scaleControlMode) {
                case ScaleControlMode.BaseScale: {
                    var meshWidth = meshFilter_.sharedMesh.bounds.extents.x * 2f;
                    var baseWidth = meshWidth * basePixel;
                    return window.width / baseWidth;
                }
                case ScaleControlMode.FixedWidth: {
                    return transform.localScale.x;
                }
                case ScaleControlMode.FixedHeight: {
                    return transform.localScale.y * window.width / window.height;
                }
                case ScaleControlMode.Manual: {
                    return transform.localScale.x;
                }
            }

            return 0f;
        }
    }

    public float height 
    {
        get 
        {
            if (window == null) return 0f;

            switch (scaleControlMode) {
                case ScaleControlMode.BaseScale: {
                    var meshHeight = meshFilter_.sharedMesh.bounds.extents.y * 2f;
                    var baseHeight = meshHeight * basePixel;
                    return window.height / baseHeight;
                }
                case ScaleControlMode.FixedWidth: {
                    return transform.localScale.x * window.height / window.width;
                }
                case ScaleControlMode.FixedHeight: {
                    return transform.localScale.y;
                }
                case ScaleControlMode.Manual: {
                    return transform.localScale.y;
                }
            }

            return 0f;
        }
    }

    bool isValid
    {
        get
        {
            return window != null && window.isValid;
        }
    }

    Material material_;
    Renderer renderer_;
    MeshFilter meshFilter_;
    Collider collider_;

    void Awake()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material; // clone
        meshFilter_ = GetComponent<MeshFilter>();
        collider_ = GetComponent<Collider>();

        list_.Add(this);
    }

    void OnDestroy()
    {
        list_.Remove(this);
    }

    void Update()
    {
        UpdateFindTargetWindowByTitle();

        if (window == null) {
            material_.mainTexture = null;
            return;
        }

        UpdateTexture();
        UpdateRenderer();
        UpdateScale();

        if (captureRequestTiming == CaptureRequestTiming.EveryFrame) {
            RequestCapture();
        }

        captureTimer_ += Time.deltaTime;

        UpdateBasicComponents();
    }

    void OnWillRenderObject()
    {
        if (captureRequestTiming == CaptureRequestTiming.OnlyWhenVisible) {
            RequestCapture();
        }
    }

    void UpdateTexture()
    {
        if (window == null) return;

        window.cursorDraw = cursorDraw;

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

    void UpdateScale()
    {
        if (window == null || window.isChild) return;

        transform.localScale = new Vector3(width, height, 1f);
    }

    void UpdateFindTargetWindowByTitle()
    {
        if (isPartialWindowTitleChanged_ || window == null) {
            isPartialWindowTitleChanged_ = false;
            window = UwcManager.Find(partialWindowTitle);
        }
    }

    void UpdateBasicComponents()
    {
        if (renderer_) renderer_.enabled = isValid;
        if (collider_) collider_.enabled = isValid;
    }

    void OnCaptured()
    {
        hasBeenCaptured_ = true;
    }

    public void RequestCapture()
    {
        if (window == null) return;

        window.captureMode = captureMode;

        float T = 1f / captureFrameRate;
        if (captureTimer_ < T) return;

        while (captureTimer_  > T) {
            captureTimer_ -= T;
        }

        var priority = capturePriority;
        if (priority == CapturePriority.Auto) {
            priority = CapturePriority.Low;
            if (window == UwcManager.cursorWindow) {
                priority = CapturePriority.High;
            } else if (window.zOrder < UwcSetting.MiddlePriorityMaxZ) {
                priority = CapturePriority.Middle;
            }
        }

        window.RequestCapture(priority);
    }
}

}