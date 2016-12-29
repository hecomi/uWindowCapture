using UnityEngine;
using uWindowCapture;
using System.Collections.Generic;

public class ExampleCreateWindows : MonoBehaviour
{
    [SerializeField] GameObject windowPrefab;
    [SerializeField] float baseWidth = 10000f;
    Dictionary<System.IntPtr, ExampleRenderWindow> windows_ = new Dictionary<System.IntPtr, ExampleRenderWindow>();

    void Update()
    {
        AddWindows();
        RemoveWindows();
        UpdateWindows();
    }

    void AddWindows()
    {
        var enumerator = Manager.windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            if (!windows_.ContainsKey(enumerator.Current.Key)) {
                var obj = Instantiate(windowPrefab, transform) as GameObject;
                var renderer = obj.GetComponent<ExampleRenderWindow>();
                if (renderer) {
                    renderer.window = enumerator.Current.Value;
                    windows_.Add(enumerator.Current.Key, renderer);
                } else {
                    DestroyImmediate(obj);
                }
            }
        }
    }

    void RemoveWindows()
    {
        var inactiveHandles = new List<System.IntPtr>();
        var enumerator = windows_.GetEnumerator();
        while (enumerator.MoveNext()) {
            var renderer = enumerator.Current.Value;
            if (!renderer.window.alive) {
                inactiveHandles.Add(enumerator.Current.Key);
            }
        }

        foreach (var handle in inactiveHandles) {
            windows_.Remove(handle);
        }
    }

    void UpdateWindows()
    {
        var pos = Vector3.zero;
        var preWidth = 0f;
        var enumerator = windows_.GetEnumerator();
        while (enumerator.MoveNext()) {
            var w = enumerator.Current.Value.window;
            var t = enumerator.Current.Value.transform;
            var width = w.width / baseWidth;
            var height = w.height / baseWidth;
            t.localEulerAngles = new Vector3(-90f, 0f, 0f);
            t.localScale = new Vector3(width, 1f, height);
            pos += new Vector3(10 * (preWidth + width) / 2, 0f, 0f);
            t.position = pos;
            preWidth = width;
        }
    }
}
