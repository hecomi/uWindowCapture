using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

public class UwcDesktopLayouter : UwcLayouter
{
    [SerializeField] 
    [Tooltip("meter / 1000 pixel")]
    float scale = 1f;

    public override void Layout(Dictionary<System.IntPtr, UwcWindowObject> windows)
    {
        var pos = Vector3.zero;
        var preWidth = 0f;

        var enumerator = windows.GetEnumerator();
        while (enumerator.MoveNext()) {
            if (enumerator.Current.Value == null) continue;
            var window = enumerator.Current.Value.window;
            var transform = enumerator.Current.Value.transform;
            var baseWidth = transform.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x * 1000f / scale;

            var title = window.title;
            transform.name = !string.IsNullOrEmpty(title) ? title : "-No Name-";

            var width = window.width / baseWidth;
            var height = window.height / baseWidth;
            var offset = new Vector3(10 * (preWidth + width) / 2, 0f, 0f);

            if (window.owner == System.IntPtr.Zero) {
                transform.localScale = new Vector3(width, 1f, height);
                pos += offset;
                transform.position = pos;
            } else {
                if (windows.ContainsKey(window.owner)) {
                    var owner = windows[window.owner];
                    transform.localPosition = new Vector3(0f, 0.1f, 0f);
                    transform.localRotation = Quaternion.identity;
                    transform.localScale = (new Vector3(width / owner.transform.localScale.x, 1f, height / owner.transform.localScale.z));
                }
            }

            preWidth = width;
        }
    }
}

}