using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTexture))]
public class UwcWindowTextureChildrenManager : MonoBehaviour 
{
    public GameObject childPrefab;

    [Tooltip("Distance per z-order")]
    public float zDistance = 0.02f;

    UwcWindowTexture windowTexture_;
    Dictionary<int, UwcWindowTexture> children = new Dictionary<int, UwcWindowTexture>();

    void Awake()
    {
        windowTexture_ = GetComponent<UwcWindowTexture>();
        windowTexture_.onWindowChanged.AddListener(OnWindowChanged);
        OnWindowChanged(windowTexture_.window, null);
    }

    void Update()
    {
        UpdateChildren();
    }

    UwcWindowTexture InstantiateChild()
    {
        var prefabChildrenManager = childPrefab.GetComponent<UwcWindowTextureChildrenManager>();
        var childTexture = Instantiate(childPrefab, transform);
        var childWindowTexture = childTexture.GetComponent<UwcWindowTexture>();
        var childrenManager = childTexture.GetComponent<UwcWindowTextureChildrenManager>();
        if (prefabChildrenManager && childrenManager) {
            childrenManager.childPrefab = prefabChildrenManager.childPrefab;
        }
        return childWindowTexture;
    }

    void OnWindowChanged(UwcWindow newWindow, UwcWindow oldWindow)
    {
        if (newWindow == oldWindow) return;

        if (oldWindow != null) {
            oldWindow.onChildAdded.RemoveListener(OnChildAdded);
            oldWindow.onChildRemoved.RemoveListener(OnChildRemoved);

            foreach (var kv in children) {
                var windowTexture = kv.Value;
                Destroy(windowTexture.gameObject);
            }

            children.Clear();
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

        var childWindowTexture = InstantiateChild();
        childWindowTexture.window = window;
        childWindowTexture.parent = windowTexture_;
        childWindowTexture.manager = windowTexture_.manager;
        childWindowTexture.scale = windowTexture_.scale;

        children.Add(window.id, childWindowTexture);
    }

    void OnChildRemoved(UwcWindow window)
    {
        UwcWindowTexture child;
        children.TryGetValue(window.id, out child);
        if (child) {
            Destroy(child.gameObject);
            children.Remove(window.id);
        }
    }

    void MoveAndScaleChildWindow(UwcWindowTexture child)
    {
        var window = child.window;
        var basePixel = child.basePixel;

        var parentDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window.parentWindow, basePixel);
        var childDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(window, basePixel);
        var localPos = childDesktopPos - parentDesktopPos;
        localPos.x /= windowTexture_.width;
        localPos.y /= windowTexture_.height;
        localPos.z = zDistance * (window.zOrder - window.parentWindow.zOrder) / transform.localScale.z;
        child.transform.localPosition = localPos;

        var worldScale = new Vector3(
            child.width / windowTexture_.width, 
            child.height / windowTexture_.height,
            1f / transform.localScale.z);
        child.transform.localScale = worldScale;
    }

    void UpdateChildren()
    {
        foreach (var kv in children) {
            var windowTexture = kv.Value;
            MoveAndScaleChildWindow(windowTexture);
        }
    }
}

}