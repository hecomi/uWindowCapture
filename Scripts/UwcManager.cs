using UnityEngine;
using UnityEngine.Events;
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

    public class WindowAddedEvent : UnityEvent<Window> {}
    private WindowAddedEvent onWindowAdded_ = new WindowAddedEvent();
    public static WindowAddedEvent onWindowAdded
    {
        get { return instance.onWindowAdded_; }
    }

    public class WindowRemovedEvent : UnityEvent<Window> {}
    private WindowRemovedEvent onWindowRemoved_ = new WindowRemovedEvent();
    public static WindowRemovedEvent onWindowRemoved
    {
        get { return instance.onWindowRemoved_; }
    }

    System.IntPtr renderEventFunc_;

    Dictionary<int, Window> windows_ = new Dictionary<int, Window>();
    static public Dictionary<int, Window> windows
    {
        get { return instance.windows_; }
    }

    int cursorWindowId_ = -1;
    static public Window cursorWindow
    {
        get { return Find(instance.cursorWindowId_); }
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
            GL.IssuePluginEvent(renderEventFunc_, 0);
            Lib.TriggerGpuUpload();
        }
    }

    void Update()
    {
        Lib.Update();
        UpdateWindowInfo();
        UpdateMessages();
    }

    void UpdateWindowInfo()
    {
        cursorWindowId_ = Lib.GetWindowIdUnderCursor();
    }

    Window FindParent(int id)
    {
        var parentId = Lib.GetWindowParentId(id);
        if (parentId == -1) return null;

        Window parent;
        windows.TryGetValue(parentId, out parent);
        return parent;
    }

    Window AddWindow(System.IntPtr handle, int id)
    {
        var parent = FindParent(id);
        var window = new Window(handle, id, parent);
        windows.Add(id, window);
        if (parent != null) {
            parent.onChildAdded.Invoke(window);
        }
        return window;
    }

    void UpdateMessages()
    {
        var messages = Lib.GetMessages();

        for (int i = 0; i < messages.Length; ++i) {
            var message = messages[i];
            var id = message.windowId;
            var handle = message.windowHandle;
            switch (message.type) {
                case MessageType.WindowAdded: {
                    var window = AddWindow(handle, id);
                    onWindowAdded.Invoke(window);
                    break;
                }
                case MessageType.WindowRemoved: {
                    var window = Find(id);
                    if (window != null) {
                        window.isAlive = false;
                        if (window.parentWindow != null) {
                            window.parentWindow.onChildRemoved.Invoke(window);
                        }
                        onWindowRemoved.Invoke(window);
                        windows.Remove(id);
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
                default: {
                    break;
                }
            }
        }
    }

    static public Window Find(int id)
    {
        Window window = null;
        windows.TryGetValue(id, out window);
        return window;
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