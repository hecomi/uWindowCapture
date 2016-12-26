using UnityEngine;
using uWindowCapture;
using System.Collections;

public class Test : MonoBehaviour
{
    public static event Lib.DebugLogDelegate onDebugLog = msg => Debug.Log(msg);
    public static event Lib.DebugLogDelegate onDebugErr = msg => Debug.LogError(msg);

    public Texture2D texture;

    void Awake()
    {
        Lib.SetDebugMode(DebugMode.File);
        Lib.SetLogFunc(onDebugLog);
        Lib.SetErrorFunc(onDebugErr);
        Lib.Initialize();
    }

    void OnApplicationQuit()
    {
        Lib.Finalize();
    }

    void OnEnable()
    {
        Lib.SetLogFunc(onDebugLog);
    }

    void OnDisable()
    {
        Lib.SetLogFunc(null);
        Lib.SetErrorFunc(null);
    }

    /*
    IEnumerator Start()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(Lib.GetRenderEventFunc(), 0);
        }
    }
    */

    void Update()
    {
        var hwnd = User32.GetForegroundWindow();
        if (hwnd != System.IntPtr.Zero) {
            /*
            var ptr = (texture) ? texture.GetNativeTexturePtr() : System.IntPtr.Zero;
            Lib.SetWindowTexture(ptr);
            if (!texture || texture.width != Lib.GetWidth() || texture.height != Lib.GetHeight()) {
                if (texture) DestroyImmediate(texture);
                texture = new Texture2D(Lib.GetWidth(), Lib.GetHeight(), TextureFormat.BGRA32, false);
            }
            */
            int width = -1, height = -1;
            var ptr = Lib.GetWindowTexture(ref width, ref height);
            if (!texture || texture.width != width || texture.height != height) {
                if (texture) DestroyImmediate(texture);
                if (ptr != System.IntPtr.Zero)
                {
                    Debug.Log(width);
                    // texture = Texture2D.CreateExternalTexture(width, height, TextureFormat.BGRA32, false, false, ptr);
                    // texture.UpdateExternalTexture(ptr);
                }
            }

            GetComponent<Renderer>().material.mainTexture = texture;
        }
    }
}
