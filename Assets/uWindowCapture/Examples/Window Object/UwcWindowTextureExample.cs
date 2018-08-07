using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTexture))]
public class UwcWindowTextureExample : MonoBehaviour
{
    UwcWindowTexture windowTexture_;
    string target_ = "";

    [SerializeField] 
    string target = "";

    [SerializeField] 
    [Tooltip("Scale per 1000 px")]
    float scale = 1f;

    void Start()
    {
        windowTexture_ = GetComponent<UwcWindowTexture>();
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
            windowTexture_.window = null;
        }

        if (windowTexture_.window == null) {
            windowTexture_.window = UwcManager.Find(target);
        }
    }

    void UpdateWindow()
    {
        if (windowTexture_.window == null) return;

        var scalePerPixel = scale / UwcSetting.BasePixel;
        var width = windowTexture_.window.width * scalePerPixel;
        var height = windowTexture_.window.height * scalePerPixel;
        transform.localScale = new Vector3(width, height, 1f);
    }
}

}