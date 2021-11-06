using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTextureManager))]
public class UwcDesktopLayouter : MonoBehaviour
{
    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    [SerializeField] 
    [Tooltip("z-margin distance between windows")]
    float zMargin = 0.1f;

    [SerializeField] 
    [Tooltip("Use position filter")]
    bool usePositionFilter = true;

    [SerializeField] 
    [Tooltip("Use scale filter")]
    bool useScaleFilter = false;

    [SerializeField] 
    [Tooltip("Smoothing filter")]
    float filter = 0.3f;

    float basePixel
    {
        get { return 1000f / scale; }
    }

    UwcWindowTextureManager manager_;

    void Awake()
    {
        manager_ = GetComponent<UwcWindowTextureManager>();
        manager_.onWindowTextureAdded.AddListener(InitWindow);
    }

    void InitWindow(UwcWindowTexture windowTexture)
    {
        MoveWindow(windowTexture, false);

        if (useScaleFilter) {
            windowTexture.transform.localScale = Vector3.zero;
        } else {
            ScaleWindow(windowTexture, false);
        }
    }

    void Update()
    {
        foreach (var kv in manager_.windows) {
            var windowTexture = kv.Value;
            CheckWindow(windowTexture);
            MoveWindow(windowTexture, usePositionFilter);
            ScaleWindow(windowTexture, useScaleFilter);
        }
    }

    void CheckWindow(UwcWindowTexture windowTexture)
    {
        windowTexture.enabled = !windowTexture.window.isIconic;
    }

    void MoveWindow(UwcWindowTexture windowTexture, bool useFilter)
    {
        var window = windowTexture.window;
        var pos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window, basePixel);
        pos.z = window.zOrder * zMargin;
        var targetPos = transform.localToWorldMatrix.MultiplyPoint3x4(pos);
        windowTexture.transform.position = (useFilter ? 
            Vector3.Slerp(windowTexture.transform.position, targetPos, filter) :
            targetPos);
    }

    void ScaleWindow(UwcWindowTexture windowTexture, bool useFilter)
    {
        windowTexture.scaleControlType = WindowTextureScaleControlType.BaseScale;
        windowTexture.scalePer1000Pixel = scale;
    }
}

}