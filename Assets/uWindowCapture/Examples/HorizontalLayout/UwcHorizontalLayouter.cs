using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcHorizontalLayouter : UwcLayouter
{
    const int WINDOW_BASE_PIXEL = 1000;

    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    void MoveAndScaleChildWindow(UwcWindowObject windowObject, float width, float height)
    {
        var window = windowObject.window;
        var windowTransform = windowObject.transform;

        var parentTransform = windowTransform.parent;
        var parentDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window.parentWindow, WINDOW_BASE_PIXEL);
        var desktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window, WINDOW_BASE_PIXEL);
        var localPos = desktopPos - parentDesktopPos;
        localPos.x *= 2f / parentTransform.lossyScale.x * transform.localScale.x;
        localPos.y *= 2f / parentTransform.lossyScale.y * transform.localScale.y;
        localPos.z = (window.zOrder - window.parentWindow.zOrder) * 0.1f;
        windowTransform.localPosition = localPos;

        var toLocalMatrix = parentTransform.worldToLocalMatrix * transform.localToWorldMatrix;
        windowTransform.localScale = toLocalMatrix.MultiplyVector(new Vector3(width, height, 1f));
    }

    public override void UpdateLayout(Dictionary<System.IntPtr, UwcWindowObject> windows)
    {
        var pos = Vector3.zero;
        var preWidth = 0f;

        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var windowObject = enumerator.Current.Value;
            var window = windowObject.window;

            var baseWidth = windowObject.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x * WINDOW_BASE_PIXEL / scale;
            var width = window.width / baseWidth;
            var height = window.height / baseWidth;

            if (window.parentWindow != null) {
                MoveAndScaleChildWindow(windowObject, width, height);
            } else {
                windowObject.transform.localScale = new Vector3(width, height, 1f);
                pos += new Vector3((preWidth + width) / 2, 0f, 0f);
                windowObject.transform.localPosition = pos;
            }

            preWidth = width;
        }
    }
}

}