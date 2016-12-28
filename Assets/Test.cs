using UnityEngine;
using uWindowCapture;
using System.Collections;
using System.Runtime.InteropServices;

public class Test : MonoBehaviour
{
    public static event Lib.DebugLogDelegate onDebugLog = msg => Debug.Log(msg);
    public static event Lib.DebugLogDelegate onDebugErr = msg => Debug.LogError(msg);

    public Texture2D texture;

    int id { get; set; }

    void Awake()
    {
        Lib.SetDebugMode(DebugMode.File);
        Lib.Initialize();
    }

    void OnApplicationQuit()
    {
        Lib.Finalize();
    }

    void OnEnable()
    {
        Lib.SetLogFunc(onDebugLog);
        Lib.SetErrorFunc(onDebugErr);
        id = Lib.AddWindow(User32.GetForegroundWindow());
    }

    void OnDisable()
    {
        Lib.SetLogFunc(null);
        Lib.SetErrorFunc(null);
        Lib.RemoveWindow(id);
    }

    IEnumerator Start()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(Lib.GetRenderEventFunc(), id);
        }
    }

    void Update()
    {
        Lib.UpdateWindowList();

        var count = Lib.GetWindowCount();
        var ptr = Lib.GetWindowList();
        var size = Marshal.SizeOf(typeof(WindowInfo));
            Debug.Log(count);

        for (int i = 0; i < count; ++i) {
            var data = new System.IntPtr(ptr.ToInt64() + size * i);
            var info = (WindowInfo)Marshal.PtrToStructure(data, typeof(WindowInfo));
            Debug.Log(info.title);
        }

        int width = Lib.GetWidth(id);
        int height = Lib.GetHeight(id);

        if (!texture || texture.width != width || texture.height != height) {
            if (texture) DestroyImmediate(texture);
            Debug.Log(width + ", " + height);
            texture = new Texture2D(width, height, TextureFormat.BGRA32, false);
            Lib.SetTexturePtr(id, texture.GetNativeTexturePtr());
            GetComponent<Renderer>().material.mainTexture = texture;
        }
    }
}
