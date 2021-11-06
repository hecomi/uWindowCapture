using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcWindowTextureManager : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;

    Dictionary<int, UwcWindowTexture> windows_ = new Dictionary<int, UwcWindowTexture>();
    public Dictionary<int, UwcWindowTexture> windows
    {
        get { return windows_; }
    }

    UwcWindowTextureEvent onWindowTextureAdded_ = new UwcWindowTextureEvent();
    public UwcWindowTextureEvent onWindowTextureAdded 
    { 
        get { return onWindowTextureAdded_; }
    }

    UwcWindowTextureEvent onWindowTextureRemoved_ = new UwcWindowTextureEvent();
    public UwcWindowTextureEvent onWindowTextureRemoved 
    { 
        get { return onWindowTextureRemoved_; }
    }

    public UwcWindowTexture AddWindowTexture(UwcWindow window)
    {
        if (!windowPrefab) {
            Debug.LogError("windowPrefab is null.");
            return null;
        }

        var obj = Instantiate(windowPrefab, transform);
        var windowTexture = obj.GetComponent<UwcWindowTexture>();
        Assert.IsNotNull(windowTexture, "Prefab must have UwcWindowTexture component.");
        windowTexture.window = window;
        windowTexture.manager = this;

        windows_.Add(window.id, windowTexture);
        onWindowTextureAdded.Invoke(windowTexture);

        return windowTexture;
    }

    public void RemoveWindowTexture(UwcWindow window)
    {
        UwcWindowTexture windowTexture;
        windows_.TryGetValue(window.id, out windowTexture);
        if (windowTexture) {
            onWindowTextureRemoved.Invoke(windowTexture);
            windows_.Remove(window.id);
            Destroy(windowTexture.gameObject);
        }
    }

    public UwcWindowTexture Get(int id)
    {
        UwcWindowTexture window = null;
        windows.TryGetValue(id, out window);
        return window;
    }
}

}