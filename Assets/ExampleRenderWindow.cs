using UnityEngine;
using uWindowCapture;

public class ExampleRenderWindow : MonoBehaviour
{
    public Window window { get; set; }

    void Update()
    {
        GetComponent<Renderer>().material.mainTexture = window.texture;
        window.shouldBeUpdated = Random.Range(0, 10) == 0;
    }
}