using UnityEngine;

namespace uWindowCapture
{

public class UwcCursorObject : MonoBehaviour 
{
    UwcCursor cursor
    {
        get { return UwcManager.cursor; }
    }

    Renderer renderer_;
    Material material_;

    void Awake()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material; // clone
        cursor.onTextureChanged.AddListener(OnTextureChanged);
    }

    void Update()
    {
        cursor.CreateTexture();
    }

    void OnTextureChanged()
    {
        material_.mainTexture = cursor.texture;
    }
}

}