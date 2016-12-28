using UnityEngine;

public class Test : MonoBehaviour
{
    uWindowCapture.Window window = null;

    void Update()
    {
        if (window == null || !window.alive) {
            window = uWindowCapture.Manager.Find("Visual");
        }

        if (window != null) {
            GetComponent<Renderer>().material.mainTexture = window.texture;
            window.shouldBeUpdated = true;
        }
    }
}
