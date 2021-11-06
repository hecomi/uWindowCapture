using UnityEngine;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowTextureManager))]
public class UwcHorizontalLayouter : MonoBehaviour
{
    UwcWindowTextureManager manager_;

    void Awake()
    {
        manager_ = GetComponent<UwcWindowTextureManager>();
    }

    void Update()
    {
        var pos = Vector3.zero;

        foreach (var kv in manager_.windows) {
            var windowTexture = kv.Value;
            var width = windowTexture.transform.localScale.x;
            pos += new Vector3(width * 0.5f, 0f, 0f);
            windowTexture.transform.localPosition = pos;
            pos += new Vector3(width * 0.5f, 0f, 0f);
        }
    }
}

}