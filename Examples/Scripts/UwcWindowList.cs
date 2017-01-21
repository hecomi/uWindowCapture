using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowList : MonoBehaviour 
{
    [SerializeField] GameObject windowListItem;
    [SerializeField] Transform listRoot;
    Dictionary<System.IntPtr, UwcWindowListItem> items_ = new Dictionary<System.IntPtr, UwcWindowListItem>();

    void Start()
    {
        UwcManager.onWindowAdded.AddListener(OnWindowAdded);
        UwcManager.onWindowRemoved.AddListener(OnWindowRemoved);

        foreach (var pair in UwcManager.windows) {
            OnWindowAdded(pair.Value);
        }
    }

    void OnWindowAdded(Window window)
    {
        if (!window.isAltTabWindow) return;

        var gameObject = Instantiate(windowListItem, listRoot, false);
        var listItem = gameObject.GetComponent<UwcWindowListItem>();
        listItem.window = window;
        items_.Add(window.handle, listItem);

        window.RequestCapture(CapturePriority.Low);
    }

    void OnWindowRemoved(System.IntPtr handle)
    {
        UwcWindowListItem listItem;
        items_.TryGetValue(handle, out listItem);
        if (listItem) {
            Destroy(listItem.gameObject);
        }
    }
}

}