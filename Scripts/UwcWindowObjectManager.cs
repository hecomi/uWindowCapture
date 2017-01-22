using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowObjectManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

    [SerializeField] bool showOnlyAltTabWindow = true;

    Dictionary<System.IntPtr, UwcWindowObject> windows_ = new Dictionary<System.IntPtr, UwcWindowObject>();
    public Dictionary<System.IntPtr, UwcWindowObject> windows
    {
        get { return windows_; }
    }

    void Start()
    {
        UwcManager.onWindowAdded.AddListener(OnWindowAdded);
        UwcManager.onWindowRemoved.AddListener(OnWindowRemoved);

        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void AddWindowObject(Window window, UwcWindowObject parent)
    {
        if (!windowPrefab) return;

        if (showOnlyAltTabWindow && (window.isRoot && !window.isAltTabWindow)) return;

        var parentTransform = parent ? parent.transform : transform; 
        var obj = Instantiate(windowPrefab, parentTransform) as GameObject;
        obj.name = window.title;

        var windowObject = obj.GetComponent<UwcWindowObject>();
        Assert.IsNotNull(windowObject, "Prefab must have UwcWindowObject component.");
        windowObject.window = window;
        windowObject.parent = parent;

        var layouters = GetComponents<UwcLayouter>();
        for (int i = 0; i < layouters.Length; ++i) {
            if (!layouters[i].enabled) continue;
            layouters[i].InitWindow(windowObject);
        }

        windows_.Add(window.handle, windowObject);
    }

    void OnWindowAdded(Window window)
    {
        if (window.isDesktop) return;

        if (window.parentWindow != null) {
            UwcWindowObject parent;
            windows.TryGetValue(window.parentWindow.handle, out parent);
            AddWindowObject(window, parent);
        } else if (window.isVisible) {
            AddWindowObject(window, null);
        }
    }

    void RemoveChildWindowsRecursively(System.IntPtr handle, Transform transform)
    {
        for (int i = 0; i < transform.childCount; ++i) {
            var child = transform.GetChild(i);
            var windowObject = child.GetComponent<UwcWindowObject>();
            if (windowObject) {
                RemoveChildWindowsRecursively(windowObject.window.handle, child);
            }
        }
        windows_.Remove(handle);
    }

    void OnWindowRemoved(System.IntPtr handle)
    {
        UwcWindowObject windowObject;
        windows_.TryGetValue(handle, out windowObject);
        if (windowObject) {
            RemoveChildWindowsRecursively(handle, windowObject.transform);
            Destroy(windowObject.gameObject);
        }
    }
}

}