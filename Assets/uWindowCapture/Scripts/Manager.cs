using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace uWindowCapture
{

public class Manager : MonoBehaviour
{
    private static Manager instance_;
    public static Manager instance 
    {
        get { return CreateInstance(); }
    }

    public static Manager CreateInstance()
    {
        if (instance_ != null) return instance_;

        var manager = FindObjectOfType<Manager>();
        if (manager) {
            instance_ = manager;
            return manager;
        }

        var go = new GameObject("uWindowCapture");
        instance_ = go.AddComponent<Manager>();
        return instance_;
    }

    public DebugMode debugMode = DebugMode.File;
    public static event Lib.DebugLogDelegate onDebugLog = msg => Debug.Log(msg);
    public static event Lib.DebugLogDelegate onDebugErr = msg => Debug.LogError(msg);

    public delegate void Event(Window window);

    public static Event onWindowAdded
    {
        get;
        set;
    }

    public static Event onWindowRemoved
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
                case MessageType.WindowAdded:
                {
                    var window = new Window(message.windowHandle, message.windowId);
                    windows.Add(message.windowHandle, window);
                    if (onWindowAdded != null) onWindowAdded(window);
                    break;
                }
                case MessageType.WindowRemoved:
                {
                    Window window;
                    windows.TryGetValue(message.windowHandle, out window);
                    if (window != null) {
                        window.alive = false;
                        if (onWindowRemoved != null) onWindowRemoved(window);
                        windows.Remove(message.windowHandle);
                    }
                    break;
                }
                default:
                {
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