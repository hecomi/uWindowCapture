using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowObject))]
public class UwcWindowObjectExample : MonoBehaviour
{
    UwcWindowObject windowObject_;

    string target_ = "";

    [SerializeField] 
    string target = "";

    [SerializeField] 
    [Tooltip("Scale per 1000 px")]
    float scale = 1f;

    void Start()
    {
        windowObject_ = GetComponent<UwcWindowObject>();
    }

    void Update()
    {
        UpdateTargetChange();
        UpdateWindow();
    }

    void UpdateTargetChange()
    {
        if (target_ != target) {
            target_ = target;
            windowObject_.window = null;
        }

        if (windowObject_.window == null) {
            var window = UwcManager.Find(target);
            if (window != null) {
                windowObject_.window = window;
            }
        }
    }

    void UpdateWindow()
    {
        if (windowObject_.window == null) return;

        var scalePerPixel = scale / UwcSetting.BasePixel;
        var width = windowObject_.window.width * scalePerPixel;
        var height = windowObject_.window.height * scalePerPixel;
        transform.localScale = new Vector3(width, height, 1f);
    }
}

}