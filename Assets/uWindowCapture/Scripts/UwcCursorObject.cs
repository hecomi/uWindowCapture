using UnityEngine;

namespace uWindowCapture
{

public class UwcCursorObject : MonoBehaviour 
{
    public UwcWindowObjectManager windowObjectManager;

    UwcCursor cursor
    {
        get { return UwcManager.cursor; }
    }

    [Tooltip("Window scale (meter per 1000 pixel)")]
    public float scale = 1f;
    public float basePixel
    {
        get { return 1000f / scale; }
    }

    public float width
    {
        get 
        {
            var meshWidth = meshFilter_.sharedMesh.bounds.extents.x * 2f;
            var baseWidth = meshWidth * basePixel;
            return cursor.width / baseWidth;
        }
    }

    public float height
    {
        get 
        {
            var meshHeight = meshFilter_.sharedMesh.bounds.extents.y * 2f;
            var baseHeight = meshHeight * basePixel;
            return cursor.height / baseHeight;
        }
    }

    Renderer renderer_;
    Material material_;
    MeshFilter meshFilter_;

    void Awake()
    {
        renderer_ = GetComponent<Renderer>();
        material_ = renderer_.material; // clone
        meshFilter_ = GetComponent<MeshFilter>();
        cursor.onTextureChanged.AddListener(OnTextureChanged);
    }

    void Update()
    {
        cursor.CreateTexture();

        var cursorWindow = UwcManager.cursorWindow;
        if (windowObjectManager && cursorWindow != null) {
            foreach (var windowObject in UwcWindowObject.list) {
                if (windowObject.window.id == cursorWindow.id) {
                    MoveAndScale(windowObject);
                    break;
                }
            }
        }

        // renderer_.enabled = cursor.visibility;
    }

    void OnTextureChanged()
    {
        material_.mainTexture = cursor.texture;
    }

    void MoveAndScale(UwcWindowObject windowObject)
    {
        transform.SetParent(windowObject.transform);

        var basePixel = windowObject.basePixel;
        var windowRatioX = windowObject.transform.lossyScale.x / windowObject.width;
        var windowRatioY = windowObject.transform.lossyScale.y / windowObject.height;

        var windowDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(windowObject.window, basePixel);
        var cursorDesktopPos = UwcWindowUtil.ConvertDesktopCoordToUnityPosition(cursor.x, cursor.y, cursor.width, cursor.height, basePixel);
        var localPos = cursorDesktopPos - windowDesktopPos;
        localPos.x *= windowRatioX / windowObject.transform.localScale.x;
        localPos.y *= windowRatioY / windowObject.transform.localScale.y;
        localPos.z = -0.001f;
        transform.localPosition = localPos;

        var worldToLocal = windowObject.transform.worldToLocalMatrix;
        var worldScale = new Vector3(width * windowRatioX, height * windowRatioY, 1f);
        var localScale = worldToLocal.MultiplyVector(worldScale);
        transform.localScale = localScale;

        transform.localRotation = Quaternion.identity;
    }
}

}