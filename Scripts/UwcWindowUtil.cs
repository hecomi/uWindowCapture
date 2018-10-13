using UnityEngine;

namespace uWindowCapture
{

public static class UwcWindowUtil
{
    public static Vector3 ConvertDesktopCoordToUnityPosition(int x, int y, int width, int height, float basePixel)
    {
        var w = width;
        var h = height;
        var l = x;
        var t = y;
        var cx = l + w / 2;
        var cy = t + h / 2;

        var sw = Lib.GetScreenWidth();
        var sh = Lib.GetScreenHeight();
        var sl = Lib.GetScreenX();
        var st = Lib.GetScreenY();
        var sCX = sl + sw / 2;
        var sCY = st + sh / 2;

        var unityX = (cx - sCX) / basePixel;
        var unityY = (-cy + sCY) / basePixel;
        return new Vector3(unityX, unityY, 0f);
    }

    public static Vector3 ConvertDesktopCoordToUnityPosition(UwcWindow window, float basePixel)
    {
        return ConvertDesktopCoordToUnityPosition(window.x, window.y, window.width, window.height, basePixel);
    }
}

}