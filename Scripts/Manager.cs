using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

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

    System.IntPtr renderEventFunc_;

    public Dictionary<System.IntPtr, Window> windows
    {
        get;
        private set;
    }

    public HashSet<WindowInfo> windowList
    {
        get;
        private set;
    }

    void Awake()
    {
        Lib.SetDebugMode(debugMode);
        Lib.Initialize();

        renderEventFunc_ = Lib.GetRenderEventFunc();
        windows = new Dictionary<System.IntPtr, Window>();
        windowList = new HashSet<WindowInfo>();
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
        Lib.RequestUpdateWindowList();
        UpdateWindows();
    }

    void UpdateWindows()
    {
        // At first, mark all windows as inactive.
        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var window = enumerator.Current.Value;
            window.alive = false;
        }

        windowList.Clear();

        // Check all window existence and add new windows to the list.
        var count = Lib.GetWindowCount();
        var ptr = Lib.GetWindowList();
        var size = Marshal.SizeOf(typeof(WindowInfo));

        for (int i = 0; i < count; ++i) {
            var data = new System.IntPtr(ptr.ToInt64() + (size * i));
            var info = (WindowInfo)Marshal.PtrToStructure(data, typeof(WindowInfo));
            var handle = info.handle;
            var title = info.title;

            windowList.Add(info);

            if (windows.ContainsKey(handle)) {
                var window = windows[handle];
                window.alive = true;
                window.title = title;
            } else {
                var window = new Window(handle);
                window.title = title;
                windows.Add(handle, window);
            }
        }

        // Remove all inactive windows.
        enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            var window = enumerator.Current.Value;
            if (!window.alive) {
                windows.Remove(window.handle);
            }
        }
    }

    static public Window Find(System.IntPtr handle)
    {
        if (instance.windows.ContainsKey(handle)) {
            return instance.windows[handle];
        }
        return null;
    }

    static public Window Find(string title)
    {
        var enumerator = instance.windowList.GetEnumerator();
        while (enumerator.MoveNext()) {
            var info = enumerator.Current;
            var handle = info.handle;
            if (info.title.IndexOf(title) != -1) {
                var windows = instance.windows;
                if (windows.ContainsKey(handle)) {
                    return windows[handle];
                }
            }
        }
        return null;
    }

    static public List<Window> FindAll(string title)
    {
        var list = new List<Window>();
        var enumerator = instance.windowList.GetEnumerator();
        while (enumerator.MoveNext()) {
            var info = enumerator.Current;
            var handle = info.handle;
            if (info.title.IndexOf(title) != -1) {
                var windows = instance.windows;
                if (windows.ContainsKey(handle)) {
                    list.Add(windows[handle]);
                }
            }
        }
        return list;
    }
}

}