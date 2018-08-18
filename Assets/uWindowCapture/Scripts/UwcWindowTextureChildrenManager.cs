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
        childWindowTexture.captureFrameRate = windowTexture_.captureFrameRate;
        childWindowTexture.captureRequestTiming = windowTexture_.captureRequestTiming;
        childWindowTexture.cursorDraw = windowTexture_.cursorDraw;
        childWindowTexture.scaleControlMode = ScaleControlMode.BaseScale;
        childWindowTexture.scalePer1000Pixel = windowTexture_.scalePer1000Pixel;

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
        var parent = window.parentWindow;

        var px = parent.x;
        var py = parent.y;
        var pw = parent.width;
        var ph = parent.height;
        var cx = window.x;
        var cy = window.y;
        var cw = window.width;
        var ch = window.height;
        var desktopX = (cw - pw) * 0.5f + (cx - px);
        var desktopY = (ch - ph) * 0.5f + (cy - py);
        var localX = desktopX / parent.width;
        var localY = -desktopY / parent.height;
        var localZ = zDistance * (window.zOrder - window.parentWindow.zOrder) / transform.localScale.z;
        child.transform.localPosition = new Vector3(localX, localY, localZ);

        var widthRatio = 1f * window.width / window.parentWindow.width;
        var heightRatio = 1f * window.height / window.parentWindow.height;
        child.transform.localScale = new Vector3(widthRatio, heightRatio, 1f);
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