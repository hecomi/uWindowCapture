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

        if (removeNonAltTabWindows && (window.isRoot && !window.isAltTabWindow)) return;

        if (removeNonTitleWindows && (parent == null && string.IsNullOrEmpty(window.title))) return;
 
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

        windows_.Add(window.id, windowObject);
    }

    void OnWindowAdded(Window window)
    {
        if (window.isDesktop) return;

        if (window.parentWindow != null) {
            UwcWindowObject parent;
            windows.TryGetValue(window.parentWindow.id, out parent);
            AddWindowObject(window, parent);
        } else if (window.isVisible) {
            AddWindowObject(window, null);
        }
    }

    void RemoveChildWindowsRecursively(int id, Transform transform)
    {
        for (int i = 0; i < transform.childCount; ++i) {
            var child = transform.GetChild(i);
            var windowObject = child.GetComponent<UwcWindowObject>();
            if (windowObject) {
                RemoveChildWindowsRecursively(windowObject.window.id, child);
            }
        }
        windows_.Remove(id);
    }

    void OnWindowRemoved(Window window)
    {
        UwcWindowObject windowObject;
        windows_.TryGetValue(window.id, out windowObject);
        if (windowObject) {
            RemoveChildWindowsRecursively(window.id, windowObject.transform);
            Destroy(windowObject.gameObject);
        }
    }
}

}