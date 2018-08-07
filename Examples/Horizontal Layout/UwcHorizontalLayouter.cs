using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTextureManager))]
public class UwcHorizontalLayouter : MonoBehaviour
{
    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    float basePixel
    {
        get { return 1000f / scale; }
    }

    UwcWindowTextureManager manager_;

    void Awake()
    {
        manager_ = GetComponent<UwcWindowTextureManager>();
    }

    void Update()
    {
        var pos = Vector3.zero;
        var preWidth = 0f;

        foreach (var kv in manager_.windows) {
            var windowTexture = kv.Value;
            var window = windowTexture.window;

            windowTexture.scale = scale;
            var width = windowTexture.width;
            var height = windowTexture.height;

            windowTexture.transform.localScale = new Vector3(width, height, 1f);
            pos += new Vector3((preWidth + width) / 2, 0f, 0f);
            windowTexture.transform.localPosition = pos;

            preWidth = width;
        }
    }
}

}