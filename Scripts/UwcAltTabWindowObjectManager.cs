using UnityEngine;

namespace uWindowCapture
{

public class UwcAltTabWindowObjectManager : UwcWindowObjectManager
{
    void Start()
    {
        UwcManager.onWindowAdded.AddListener(OnWindowAdded);
        UwcManager.onWindowRemoved.AddListener(OnWindowRemoved);

        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void OnWindowAdded(UwcWindow window)
    {
        if (window.parentWindow != null) return; // handled by UwcWindowObject
        if (!window.isVisible) return;
        if (!window.isAltTabWindow) return;

        AddWindowObject(window);
    }

    void OnWindowRemoved(UwcWindow window)
    {
        RemoveWindowObject(window);
    }
}

}