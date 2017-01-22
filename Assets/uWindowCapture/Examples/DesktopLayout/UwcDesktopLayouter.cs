using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcDesktopLayouter : UwcLayouter
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

    Vector3 offset
    {
        get { return new Vector3(-Lib.GetScreenWidth() / (2 * basePixel), 0f, 0f); }
    }

    void CheckWindow(UwcWindowObject windowObject)
    {
        windowObject.enabled = !windowObject.window.isIconic;
    }

    void MoveWindow(UwcWindowObject windowObject, bool useFilter)
    {
        var window = windowObject.window;
        var pos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window, basePixel);
        pos.z = window.zOrder * zMargin;
        var targetPos = transform.localToWorldMatrix.MultiplyPoint3x4(offset + pos);
        windowObject.transform.position = (useFilter ? 
            Vector3.Slerp(windowObject.transform.position, targetPos, filter) :
            targetPos);
    }

    void ScaleWindow(UwcWindowObject windowObject, bool useFilter)
    {
        var window = windowObject.window;

        var w = window.width / basePixel;
        var h = window.height / basePixel;

        var parent = windowObject.transform.parent;
        var targetWorldScale = new Vector3(w, h, 1f);
        var targetLocalScale = (parent.worldToLocalMatrix * transform.localToWorldMatrix).MultiplyVector(targetWorldScale);

        windowObject.transform.localScale = (useFilter ?
            Vector3.Slerp(windowObject.transform.localScale, targetLocalScale, filter) :
            targetLocalScale);
    }

    public override void InitWindow(UwcWindowObject windowObject)
    {
        MoveWindow(windowObject, false);

        if (useScaleFilter) {
            windowObject.transform.localScale = Vector3.zero;
        } else {
            ScaleWindow(windowObject, false);
        }
    }

    public override void UpdateLayout(Dictionary<System.IntPtr, UwcWindowObject> windows)
    {
        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var windowObject = enumerator.Current.Value;
            CheckWindow(windowObject);
            MoveWindow(windowObject, usePositionFilter);
            ScaleWindow(windowObject, useScaleFilter);
        }
    }
}

}