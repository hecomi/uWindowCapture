using UnityEngine;
using UnityEditor;

namespace uWindowCapture
{

[CustomEditor(typeof(UwcIconTexture))]
public class UwcIconTextureEditor : Editor
{
    UwcIconTexture texture
    {
        get { return target as UwcIconTexture; }
    }

    public override void OnInspectorGUI()
    {
        var windowTexture = (UwcWindowTexture)EditorGUILayout.ObjectField("Window Texture", texture.windowTexture, typeof(UwcWindowTexture), true);
        if (texture.windowTexture != windowTexture) {
            texture.windowTexture = windowTexture;
        }
    }
}

}