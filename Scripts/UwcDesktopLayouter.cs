using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcDesktopLayouter : UwcLayouter
{
    const float BASE_PIXEL = 1000f;

    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    [SerializeField] 
    [Tooltip("z-margin distance between windows")]
    float zMargin = 0.1f;

    [SerializeField] 
    [Tooltip("Smoothing filter")]
    float filter = 0.3f;

    float basePixel
    {
        get { return BASE_PIXEL / scale; }
    }

    Vector3 offset
    {
        get 
        { 
            return new Vector3(
                -Lib.GetScreenWidth() / (2 * basePixel),
                0f,
                0f);
        }
    }

    void MoveWindow(UwcWindowObject windowObject, bool useFilter)
    {
        var window = windowObject.window;

        var w = window.width / basePixel;
        var h = window.height / basePixel;
        var l = window.x / basePixel;
        var t = window.y / basePixel;
        var x = (l + w / 2);
        var y = (Screen.height / basePixel) - (t + h / 2);
        var z = window.zOrder * zMargin;

        var targetPos = offset + new Vector3(x, y, z);
        windowObject.transform.position = (useFilter ? 
            Vector3.Slerp(windowObject.transform.position, targetPos, filter) :
            targetPos);
    }

    void ScaleWindow(UwcWindowObject windowObject)
    {
        var window = windowObject.window;

        var w = window.width / basePixel;
        var h = window.height / basePixel;

        var parent = windowObject.transform.parent;
        windowObject.transform.localScale = parent.worldToLocalMatrix.MultiplyVector(new Vector3(w, h, 1f));
    }

    public override void InitWindow(UwcWindowObject windowObject)
    {
        MoveWindow(windowObject, false);
        ScaleWindow(windowObject);
    }

    public override void UpdateLayout(Dictionary<System.IntPtr, UwcWindowObject> windows)
    {
        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var windowObject = enumerator.Current.Value;
            var window = windowObject.window;

            var title = window.title;
            if (!string.IsNullOrEmpty(title)) {
                windowObject.transform.name = title;
            }

            MoveWindow(windowObject, true);
            ScaleWindow(windowObject);
        }
    }
}

}