using UnityEngine;

namespace uWindowCapture
{

public class UwcGetPixelExample : MonoBehaviour
{
    [SerializeField] UwcWindowTexture uwcTexture;

    Material material_;

    void Start()
    {
        material_ = GetComponent<Renderer>().material;
    }

    void Update()
    {
        var window = uwcTexture.window;
        if (window == null) return;

        if (UwcManager.cursorWindow == window) {
            var cursorPos = Lib.GetCursorPosition();
            var x = cursorPos.x - window.x;
            var y = cursorPos.y - window.y;
            material_.color = window.GetPixel(x, y);
        }
    }
}

}