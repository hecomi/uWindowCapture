using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowObjectManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

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

    UwcWindowObject FindParent(Window window)
    {
        if (windows_.ContainsKey(window.owner)) {
            return windows[window.owner];
        }

        if (windows_.ContainsKey(window.parent)) {
            return windows[window.parent];
        }

        foreach (var pair in windows) {
            var obj = pair.Value;
            if (!obj.window.isChild &&
                obj.window.processId == window.processId && 
                obj.window.threadId == window.threadId) {
                return obj;
            }
        }

        return null;
    }

    void AddWindowObject(Window window, UwcWindowObject parent, bool isChild)
    {
        if (!windowPrefab) return;

        var parentTransform = parent ? parent.transform : transform; 
        var obj = Instantiate(windowPrefab, parentTransform) as GameObject;
        obj.name = window.title;

        var windowObject = obj.GetComponent<UwcWindowObject>();
        Assert.IsNotNull(windowObject, "Prefab must have UwcWindowObject component.");
        windowObject.window = window;
        windowObject.parent = parent ? parent.window : null;
        windowObject.isChild = isChild;

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

        var parent = FindParent(window);
        if (parent) {
            AddWindowObject(window, parent, true);
        } else if (window.isVisible) {
            AddWindowObject(window, null, false);
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