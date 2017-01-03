using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcManager : MonoBehaviour
{
    private static UwcManager instance_;
    public static UwcManager instance 
    {
        get { return CreateInstance(); }
    }

    public static UwcManager CreateInstance()
    {
        if (instance_ != null) return instance_;

        var manager = FindObjectOfType<UwcManager>();
        if (manager) {
            instance_ = manager;
            return manager;
        }

        var go = new GameObject("uWindowCapture");
        instance_ = go.AddComponent<UwcManager>();
        return instance_;
    }

    public DebugMode debugMode = DebugMode.File;
    public static event Lib.DebugLogDelegate onDebugLog = msg => Debug.Log(msg);
    public static event Lib.DebugLogDelegate onDebugErr = msg => Debug.LogError(msg);

    public delegate void WindowAddedHandler(Window window);
    public static WindowAddedHandler onWindowAdded
    {
        get;
        set;
    }

    public delegate void WindowRemovedHandler(System.IntPtr handle);
    public static WindowRemovedHandler onWindowRemoved
    {
        get;
        set;
    }

    System.IntPtr renderEventFunc_;

    Dictionary<System.IntPtr, Window> windows_ = new Dictionary<System.IntPtr, Window>();
    static public Dictionary<System.IntPtr, Window> windows
    {
        get { return instance.windows_; }
    }

    void Awake()
    {
        Lib.SetDebugMode(debugMode);
        Lib.Initialize();
        renderEventFunc_ = Lib.GetRenderEventFunc();
    }

    void Start()
    {
        StartCoroutine(Render());
    }

    void OnApplicationQuit()
    {
        Lib.Finalize();
    }

    void OnEnable()
    {
        Lib.SetLogFunc(onDebugLog);
        Lib.SetErrorFunc(onDebugErr);
    }

    void OnDisable()
    {
        Lib.SetLogFunc(null);
        Lib.SetErrorFunc(null);
    }

    IEnumerator Render()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();

            var enumerator = windows.GetEnumerator();
            while (enumerator.MoveNext()) {
                var window = enumerator.Current.Value;
                if (window.shouldBeUpdated) {
                    window.shouldBeUpdated = false;
                    window.UpdateTextureIfNeeded();
                    GL.IssuePluginEvent(renderEventFunc_, window.id);
                }
            }
        }
    }

    void Update()
    {
        Lib.Update();
        UpdateMessages();
    }

    void UpdateMessages()
    {
        var messages = Lib.GetMessages();

        for (int i = 0; i < messages.Length; ++i) {
            var message = messages[i];
            switch (message.type) {
                case MessageType.WindowAdded: {
                    var window = new Window(message.windowHandle, message.windowId);
                    windows.Add(message.windowHandle, window);
                    if (onWindowAdded != null) onWindowAdded(window);
                    break;
                }
                case MessageType.WindowRemoved: {
                    Window window;
                    windows.TryGetValue(message.windowHandle, out window);
                    if (window != null) {
                        window.isAlive = false;
                        if (onWindowRemoved != null) onWindowRemoved(message.windowHandle);
                        windows.Remove(message.windowHandle);
                    }
                    break;
                }
                case MessageType.WindowCaptured: {
                    Window window;
                    windows.TryGetValue(message.windowHandle, out window);
                    if (window != null && window.onCaptured != null) {
                        window.onCaptured();
                    }
                    break;
                }
                case MessageType.WindowSizeChanged: {
                    Window window;
                    windows.TryGetValue(message.windowHandle, out window);
                    if (window != null && window.onSizeChanged != null) {
                        window.onSizeChanged();
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    static public Window Find(System.IntPtr handle)
    {
        if (windows.ContainsKey(handle)) {
            return windows[handle];
        }
        return null;
    }

    static public Window Find(string title)
    {
        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var window = enumerator.Current.Value;
            if (window.title.IndexOf(title) != -1) {
                return window;
            }
        }
        return null;
    }

    static public List<Window> FindAll(string title)
    {
        var list = new List<Window>();
        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var window = enumerator.Current.Value;
            if (window.title.IndexOf(title) != -1) {
                list.Add(window);
            }
        }
        return list;
    }
}

}