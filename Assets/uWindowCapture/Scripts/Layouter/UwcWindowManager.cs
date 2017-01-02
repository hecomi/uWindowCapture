using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

    Dictionary<System.IntPtr, UwcTexture> windows_ = new Dictionary<System.IntPtr, UwcTexture>();
    public Dictionary<System.IntPtr, UwcTexture> windows
    {
        get { return windows_; }
    }

    void Start()
    {
        UwcManager.onWindowAdded += OnWindowAdded;
        UwcManager.onWindowRemoved += OnWindowRemoved;

        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void OnWindowAdded(Window window)
    {
        if (!windowPrefab) return;

        if (window.isAltTabWindow || window.isDesktop) {
            var obj = Instantiate(windowPrefab, transform) as GameObject;
            var renderer = obj.GetComponent<UwcTexture>();
            if (renderer) {
                renderer.window = window;
                windows_.Add(window.handle, renderer);
            }
        } else if (window.owner != System.IntPtr.Zero) {
            if (windows_.ContainsKey(window.owner)) {
                var owner = windows_[window.owner];
                var obj = Instantiate(windowPrefab, transform) as GameObject;
                obj.transform.SetParent(owner.transform);
                var renderer = obj.GetComponent<UwcTexture>();
                if (renderer) {
                    renderer.window = window;
                    windows_.Add(window.handle, renderer);
                }
            }
        }
    }

    void OnWindowRemoved(Window window)
    {
        if (windows_.ContainsKey(window.handle)) {
            Destroy(windows_[window.handle].gameObject);
            windows_.Remove(window.handle);
        }
    }
}

}