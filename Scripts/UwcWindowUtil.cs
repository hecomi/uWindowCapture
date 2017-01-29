using UnityEngine;

namespace uWindowCapture
{

public static class UwcWindowUtil
{
    public static Vector3 ConvertDesktopCoordToUnityPosition(UwcWindow window, float basePixel)
    {
        var w = window.width / basePixel;
        var h = window.height / basePixel;
        var l = window.x / basePixel;
        var t = window.y / basePixel;
        var x = (l + w / 2);
        var y = (Screen.height / basePixel) - (t + h / 2);
        return new Vector3(x, y, 0f);
    }
}

}