using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowObjectManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

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

    public UwcWindowObject AddWindowObject(UwcWindow window)
    {
        if (!windowPrefab) {
            Debug.LogError("windowPrefab is null.");
            return null;
        }

        var obj = Instantiate(windowPrefab, transform);
        obj.name = window.title;

        var windowObject = obj.GetComponent<UwcWindowObject>();
        Assert.IsNotNull(windowObject, "Prefab must have UwcWindowObject component.");
        windowObject.window = window;
        windowObject.manager = this;

        var prefabChildrenManager = windowPrefab.GetComponent<UwcWindowObjectChildrenManager>();
        var childrenManager = windowObject.GetComponent<UwcWindowObjectChildrenManager>();
        if (prefabChildrenManager && childrenManager) {
            childrenManager.childPrefab = prefabChildrenManager.childPrefab;
        }

        windows_.Add(window.id, windowObject);
        onWindowObjectAdded.Invoke(windowObject);

        return windowObject;
    }

    public void RemoveWindowObject(UwcWindow window)
    {
        UwcWindowObject windowObject;
        windows_.TryGetValue(window.id, out windowObject);
        if (windowObject) {
            onWindowObjectRemoved.Invoke(windowObject);
            windows_.Remove(window.id);
            Destroy(windowObject.gameObject);
        }
    }

    public UwcWindowObject Get(int id)
    {
        UwcWindowObject window = null;
        windows.TryGetValue(id, out window);
        return window;
    }
}

}