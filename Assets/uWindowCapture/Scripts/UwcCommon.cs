using UnityEngine;
using UnityEngine.Events;

namespace uWindowCapture
{

public class UwcEvent : UnityEvent
{
}

public class UwcWindowEvent : UnityEvent<UwcWindow>
{
}

public class UwcWindowChangeEvent : UnityEvent<UwcWindow, UwcWindow>
{
}

public class UwcWindowTextureEvent : UnityEvent<UwcWindowTexture>
{
}

public static class UwcSetting
{
    public const int BasePixel = 1000;
    public const int MiddlePriorityMaxZ = 5;
}

}