using UnityEngine;

namespace uWindowCapture
{

public static class UwcWindowUtil
{
    public static Vector3 ConvertDesktopCoordToUnityPosition(int x, int y, int width, int height, float basePixel)
    {
        var w = width / basePixel;
        var h = height / basePixel;
        var l = x / basePixel;
        var t = y / basePixel;
        var unityX = (l + w / 2);
        var unityY = (Screen.height / basePixel) - (t + h / 2);
        return new Vector3(unityX, unityY, 0f);
    }

    public static Vector3 ConvertDesktopCoordToUnityPosition(UwcWindow window, float basePixel)
    {
        return ConvertDesktopCoordToUnityPosition(window.x, window.y, window.width, window.height, basePixel);
    }
}

}