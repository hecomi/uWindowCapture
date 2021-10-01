using UnityEngine;
using System.Collections;
using System.Collections.Generic;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace uWindowCapture
{

public class UwcManager : MonoBehaviour
{
    private static UwcManager instance_;
    public static UwcManager instance 
    {
        get { return CreateInstance(); }
    }

    private static UwcManager CreateInstance()
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

#if UNITY_EDITOR
    [MenuItem("GameObject/uWindowCapture/Manager", false, 100)]
    public static void CreateManagerGameObject()
    {
        CreateInstance();
    }
#endif

    public DebugMode debugModeFromInspector = DebugMode.File;
    private static DebugMode debugModeFromScript = DebugMode.File;
    private static bool debugModeChangedFromScript = false;
    public static DebugMode debugMode
    {
        get 
        { 
            return debugModeChangedFromScript ? 
                debugModeFromScript : 
                instance.debugModeFromInspector;
        }
        set 
        { 
            debugModeFromScript = value; 
            debugModeChangedFromScript = true; 
        }
    }

    public static event Lib.DebugLogDelegate onDebugLog = OnDebugLog;
    public static event Lib.DebugLogDelegate onDebugErr = OnDebugErr;
    [AOT.MonoPInvokeCallback(typeof(Lib.DebugLogDelegate))]
    private static void OnDebugLog(string msg) { Debug.Log(msg); }
    [AOT.MonoPInvokeCallback(typeof(Lib.DebugLogDelegate))]
    private static void OnDebugErr(string msg) { Debug.LogError(msg); }

    public WindowTitlesUpdateTiming windowTitlesUpdateTiming = WindowTitlesUpdateTiming.Manual;

    private UwcWindowEvent onWindowAdded_ = new UwcWindowEvent();
    public static UwcWindowEvent onWindowAdded
    {
        get { return instance.onWindowAdded_; }
    }

    private UwcWindowEvent onWindowRemoved_ = new UwcWindowEvent();
    public static UwcWindowEvent onWindowRemoved
    {
        get { return instance.onWindowRemoved_; }
    }

    private UwcWindowEvent onDesktopAdded_ = new UwcWindowEvent();
    public static UwcWindowEvent onDesktopAdded
    {
        get { return instance.onDesktopAdded_; }
    }

    private UwcWindowEvent onDesktopRemoved_ = new UwcWindowEvent();
    public static UwcWindowEvent onDesktopRemoved
    {
        get { return instance.onDesktopRemoved_; }
    }

    private UwcEvent onCursorCaptured_ = new UwcEvent();
    public static UwcEvent onCursorCaptured
    {
        get { return instance.onCursorCaptured_; }
    }

    System.IntPtr renderEventFunc_;

    Dictionary<int, UwcWindow> windows_ = new Dictionary<int, UwcWindow>();
    static public Dictionary<int, UwcWindow> windows
    {
        get { return instance.windows_; }
    }

    int cursorWindowId_ = -1;
    static public UwcWindow cursorWindow
    {
        get { return Find(instance.cursorWindowId_); }
    }

    UwcCursor cursor_ = new UwcCursor();
    static public UwcCursor cursor
    {
        get { return instance.cursor_; }
    }

    List<int> desktops_ = new List<int>();
    static public int desktopCount
    {
        get { return instance.desktops_.Count; }
    }

    static public bool isWindowsGraphicsCaptureSupported
    {
        get { return Lib.IsWindowsGraphicsCaptureSupported(); }
    }

    static public bool isWindowsGraphicsCaptureCursorCaptureEnabledApiSupported
    {
        get { return Lib.IsWindowsGraphicsCaptureCursorCaptureEnabledApiSupported(); }
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
        Resources.UnloadUnusedAssets();
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
            GL.IssuePluginEvent(renderEventFunc_, 0);
        }
    }

    void Update()
    {
        Lib.Update(Time.deltaTime);
        UpdateWindowInfo();
        UpdateMessages();
        UpdateWindowTitles();
    }

    void UpdateWindowInfo()
    {
        cursorWindowId_ = Lib.GetWindowIdUnderCursor();
    }

    UwcWindow AddWindow(int id)
    {
        var window = new UwcWindow(id);
        windows.Add(id, window);
        return window;
    }

    void UpdateMessages()
    {
        var messages = Lib.GetMessages();

        for (int i = 0; i < messages.Length; ++i) {
            var message = messages[i];
            var id = message.windowId;
            switch (message.type) {
                case MessageType.WindowAdded: {
                    var window = AddWindow(id);
                    if (window.isAlive && window.isDesktop) {
                        desktops_.Add(id);
                        onDesktopAdded.Invoke(window);
                    } else {
                        onWindowAdded.Invoke(window);
                    }
                    break;
                }
                case MessageType.WindowRemoved: {
                    var window = Find(id);
                    if (window != null) {
                        window.isAlive = false;
                        if (window.parentWindow != null) {
                            window.parentWindow.onChildRemoved.Invoke(window);
                        }
                        windows.Remove(id);
                        if (window.isAlive && window.isDesktop) {
                            desktops_.Remove(id);
                            onDesktopRemoved.Invoke(window);
                        } else {
                            onWindowRemoved.Invoke(window);
                        }
                    }
                    break;
                }
                case MessageType.WindowCaptured: {
                    var window = Find(id);
                    if (window != null) {
                        window.onCaptured.Invoke();
                    }
                    break;
                }
                case MessageType.WindowSizeChanged: {
                    var window = Find(id);
                    if (window != null) {
                        window.onSizeChanged.Invoke();
                    }
                    break;
                }
                case MessageType.IconCaptured: {
                    var window = Find(id);
                    if (window != null) {
                        window.onIconCaptured.Invoke();
                    }
                    break;
                }
                case MessageType.CursorCaptured: {
                    cursor.onCaptured.Invoke();
                    break;
                }
                case MessageType.TextureNullError: {
                    var window = Find(id);
                    if (window != null) {
                        window.ResetWindowTexture();
                    }
                    break;
                }
                case MessageType.TextureSizeError: {
                    var window = Find(id);
                    if (window != null) {
                        window.ResetWindowTexture();
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    void UpdateWindowTitles()
    {
        switch (windowTitlesUpdateTiming) {
            case WindowTitlesUpdateTiming.Manual:
                break;
            case WindowTitlesUpdateTiming.AlwaysAllWindows:
                UpdateAllWindowTitles();
                break;
            case WindowTitlesUpdateTiming.AlwaysAltTabWindows:
                UpdateAltTabWindowTitles();
                break;
        }
    }

    static public UwcWindow Find(int id)
    {
        UwcWindow window = null;
        windows.TryGetValue(id, out window);
        return window;
    }

    static public UwcWindow Find(string partialTitle, bool isAltTabWindow = true)
    {
        UwcWindow target = null;
        int minIndex = int.MaxValue;
        foreach (var kv in windows) {
            var window = kv.Value;
            if (isAltTabWindow && !window.isAltTabWindow) {
                continue;
            }
            var index = window.title.IndexOf(partialTitle);
            if (index == 0) {
                return window;
            } else if (index != -1 && index < minIndex) {
                minIndex = index;
                target = window;
            }
        }
        return target;
    }

    static public UwcWindow Find(System.IntPtr handle)
    {
        foreach (var kv in windows) {
            var window = kv.Value;
            if (window.handle == handle) {
                return window;
            }
        }
        return null;
    }

    static public UwcWindow Find(System.Func<UwcWindow, bool> func)
    {
        foreach (var kv in windows) {
            var window = kv.Value;
            if (func(window)) return window;
        }
        return null;
    }

    static public List<UwcWindow> FindAll(string title)
    {
        var list = new List<UwcWindow>();
        foreach (var kv in windows) {
            var window = kv.Value;
            if (window.title.IndexOf(title) != -1) {
                list.Add(window);
            }
        }
        return list;
    }

    static public UwcWindow FindParent(int id)
    {
        var parentId = Lib.GetWindowParentId(id);
        if (parentId == -1) return null;

        UwcWindow parent;
        windows.TryGetValue(parentId, out parent);
        return parent;
    }

    static public UwcWindow FindDesktop(int index)
    {
        if (index < 0 || index >= desktopCount) return null;
        var id = instance.desktops_[index];
        return Find(id);
    }

    static public void UpdateAllWindowTitles()
    {
        foreach (var kv in windows) {
            var window = kv.Value;
            window.RequestUpdateTitle();
        }
    }

    static public void UpdateAltTabWindowTitles()
    {
        foreach (var kv in windows) {
            var window = kv.Value;
            if (window.isAltTabWindow) {
                window.RequestUpdateTitle();
            }
        }
    }
}

}