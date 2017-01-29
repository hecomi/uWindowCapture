using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowObjectManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;
    [SerializeField] bool removeNonAltTabWindows = true;
    [SerializeField] bool removeNonTitleWindows = true;

    Dictionary<int, UwcWindowObject> windows_ = new Dictionary<int, UwcWindowObject>();
    public Dictionary<int, UwcWindowObject> windows
    {
        get { return windows_; }
    }

    UwcWindowObjectEvent onWindowObjectAdded_ = new UwcWindowObjectEvent();
    public UwcWindowObjectEvent onWindowObjectAdded 
    { 
        get { return onWindowObjectAdded_; }
    }

    UwcWindowObjectEvent onWindowObjectRemoved_ = new UwcWindowObjectEvent();
    public UwcWindowObjectEvent onWindowObjectRemoved 
    { 
        get { return onWindowObjectRemoved_; }
    }

    void Start()
    {
        UwcManager.onWindowAdded.AddListener(OnWindowAdded);
        UwcManager.onWindowRemoved.AddListener(OnWindowRemoved);

        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void AddWindowObject(UwcWindow window)
    {
        if (!windowPrefab) return;

        var obj = Instantiate(windowPrefab, transform);
        obj.name = window.title;

        var windowObject = obj.GetComponent<UwcWindowObject>();
        Assert.IsNotNull(windowObject, "Prefab must have UwcWindowObject component.");
        windowObject.window = window;

        windows_.Add(window.id, windowObject);
        onWindowObjectAdded.Invoke(windowObject);
    }

    void OnWindowAdded(UwcWindow window)
    {
        if (window.isDesktop) return;
        if (window.parentWindow != null) return; // handled by UwcWindowObject
        if (!window.isVisible) return;

        if (removeNonAltTabWindows && !window.isAltTabWindow) return;
        if (removeNonTitleWindows && string.IsNullOrEmpty(window.title)) return;

        AddWindowObject(window);
    }

    void OnWindowRemoved(UwcWindow window)
    {
        UwcWindowObject windowObject;
        windows_.TryGetValue(window.id, out windowObject);
        if (windowObject) {
            onWindowObjectRemoved.Invoke(windowObject);
            Destroy(windowObject.gameObject);
        }
    }
}

}