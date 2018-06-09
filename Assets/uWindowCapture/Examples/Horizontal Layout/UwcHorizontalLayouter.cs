using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowObjectManager))]
public class UwcHorizontalLayouter : MonoBehaviour
{
    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    float basePixel
    {
        get { return 1000f / scale; }
    }

    UwcWindowObjectManager manager_;

    void Awake()
    {
        manager_ = GetComponent<UwcWindowObjectManager>();
    }

    void Update()
    {
        var pos = Vector3.zero;
        var preWidth = 0f;

        foreach (var kv in manager_.windows) {
            var windowObject = kv.Value;
            var window = windowObject.window;

            windowObject.scale = scale;
            var width = windowObject.width;
            var height = windowObject.height;

            windowObject.transform.localScale = new Vector3(width, height, 1f);
            pos += new Vector3((preWidth + width) / 2, 0f, 0f);
            windowObject.transform.localPosition = pos;

            preWidth = width;
        }
    }
}

}