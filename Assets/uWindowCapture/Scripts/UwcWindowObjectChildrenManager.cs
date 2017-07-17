using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowObject))]
public class UwcWindowObjectChildrenManager : MonoBehaviour 
{
    public GameObject childPrefab;

    [Tooltip("Distance per z-order")]
    public float zDistance = 0.02f;

    UwcWindowObject windowObject_;
    Dictionary<int, UwcWindowObject> children = new Dictionary<int, UwcWindowObject>();

    void Awake()
    {
        windowObject_ = GetComponent<UwcWindowObject>();
        windowObject_.onWindowChanged.AddListener(OnWindowChanged);
        OnWindowChanged(windowObject_.window, null);
    }

    void Update()
    {
        UpdateChildren();
    }

    UwcWindowObject InstantiateChild()
    {
        var prefabChildrenManager = childPrefab.GetComponent<UwcWindowObjectChildrenManager>();
        var childObject = Instantiate(childPrefab, transform);
        var childWindowObject = childObject.GetComponent<UwcWindowObject>();
        var childrenManager = childObject.GetComponent<UwcWindowObjectChildrenManager>();
        if (prefabChildrenManager && childrenManager) {
            childrenManager.childPrefab = prefabChildrenManager.childPrefab;
        }
        return childWindowObject;
    }

    void OnWindowChanged(UwcWindow newWindow, UwcWindow oldWindow)
    {
        if (oldWindow != null) {
            oldWindow.onChildAdded.RemoveListener(OnChildAdded);
            oldWindow.onChildRemoved.RemoveListener(OnChildRemoved);
        }

        if (newWindow != null) {
            newWindow.onChildAdded.AddListener(OnChildAdded);
            newWindow.onChildRemoved.AddListener(OnChildRemoved);

            foreach (var pair in UwcManager.windows) {
                var window = pair.Value;
                if (
                    !window.isAltTabWindow &&
                    window.isChild && 
                    window.parentWindow.id == newWindow.id) {
                    OnChildAdded(window);
                }
            }
        }
    }

    void OnChildAdded(UwcWindow window)
    {
        if (!childPrefab) {
            Debug.LogError("childPrefab is not set.");
            return;
        }

        var childWindowObject = InstantiateChild();
        childWindowObject.window = window;
        childWindowObject.parent = windowObject_;
        childWindowObject.manager = windowObject_.manager;
        childWindowObject.scale = windowObject_.scale;

        children.Add(window.id, childWindowObject);
    }

    void OnChildRemoved(UwcWindow window)
    {
        UwcWindowObject child;
        children.TryGetValue(window.id, out child);
        if (child) {
            Destroy(child.gameObject);
            children.Remove(window.id);
        }
    }

    void MoveAndScaleChildWindow(UwcWindowObject child)
    {
        var window = child.window;
        var basePixel = child.basePixel;

        var lossyScale = transform.lossyScale;
        var parentRatioX = lossyScale.x / windowObject_.width;
        var parentRatioY = lossyScale.y / windowObject_.height;

        var parentDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window.parentWindow, basePixel);
        var childDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window, basePixel);
        var localPos = childDesktopPos - parentDesktopPos;
        localPos.x /= windowObject_.width;
        localPos.y /= windowObject_.height;
        localPos.z = zDistance * (window.zOrder - window.parentWindow.zOrder) / transform.localScale.z;
        child.transform.localPosition = localPos;

        var worldScale = new Vector3(
            child.width / windowObject_.width, 
            child.height / windowObject_.height,
            1f / transform.localScale.z);
        child.transform.localScale = worldScale;
    }

    void UpdateChildren()
    {
        var enumerator = children.GetEnumerator();
        while (enumerator.MoveNext()) {
            var windowObject = enumerator.Current.Value;
            MoveAndScaleChildWindow(windowObject);
        }
    }
}

}