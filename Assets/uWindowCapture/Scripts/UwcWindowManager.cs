using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

    Dictionary<System.IntPtr, UwcWindowObject> windows_ = new Dictionary<System.IntPtr, UwcWindowObject>();
    public Dictionary<System.IntPtr, UwcWindowObject> windows
    {
        get { return windows_; }
    }

    void Start()
    {
        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void OnEnable()
    {
        UwcManager.onWindowAdded += OnWindowAdded;
        UwcManager.onWindowRemoved += OnWindowRemoved;
    }

    void OnDisable()
    {
        UwcManager.onWindowAdded -= OnWindowAdded;
        UwcManager.onWindowRemoved -= OnWindowRemoved;
    }

    void AddWindowObject(Window window, Transform parent)
    {
        if (!windowPrefab) return;

        var obj = Instantiate(windowPrefab, parent) as GameObject;

        var title = window.title;
        obj.name = !string.IsNullOrEmpty(title) ? title : ("-No Name- (" + window.handle.ToString() + ")");

        var windowObject = obj.GetComponent<UwcWindowObject>();
        Assert.IsNotNull(windowObject, "Prefab must have UwcWindowObject component.");
        windowObject.window = window;

        var layouters = GetComponents<UwcLayouter>();
        for (int i = 0; i < layouters.Length; ++i) {
            if (!layouters[i].enabled) continue;
            layouters[i].InitWindow(windowObject);
        }

        windows_.Add(window.handle, windowObject);
    }

    void OnWindowAdded(Window window)
    {
        if (window.isDesktop) {
            // skip
        } else if (windows_.ContainsKey(window.owner)) {
            var owner = windows_[window.owner];
            AddWindowObject(window, owner.transform);
        } else if (window.isVisible && window.isEnabled) {
            AddWindowObject(window, transform);
        } else {
            // Debug.LogFormat("Unhandled window: {0} {1}", window.handle, window.title);
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