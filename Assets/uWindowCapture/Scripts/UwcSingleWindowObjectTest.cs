using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowObject))]
public class UwcSingleWindowObjectTest : MonoBehaviour
{
    UwcWindowObject windowObject_;

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
        if (windowObject_.window == null) {
            FindWindow();
        } else {
            UpdateWindow();
        }
    }

    void FindWindow()
    {
        var window = UwcManager.Find(target);
        if (window != null) {
            windowObject_.window = window;
        }
    }

    void UpdateWindow()
    {
        var scalePerPixel = scale / UwcSetting.BasePixel;
        var width = windowObject_.window.width * scalePerPixel;
        var height = windowObject_.window.height * scalePerPixel;
        transform.localScale = new Vector3(width, height, 1f);
    }
}

}