using UnityEngine;
using uWindowCapture;

public class ExampleRenderWindow : MonoBehaviour
{
    public Window window { get; set; }
    public CaptureMode mode = CaptureMode.PrintWindow;

    void Start()
    {
        mode = window.captureMode;
    }

    void Update()
    {
        GetComponent<Renderer>().material.mainTexture = window.texture;
        window.captureMode = mode;
        window.shouldBeUpdated = Random.Range(0, 10) == 0;
    }
}