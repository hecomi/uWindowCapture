using UnityEngine;

#pragma warning disable 0414

namespace uWindowCapture
{

public class UwcRayCastExample : MonoBehaviour
{
    [SerializeField]
    Transform from;

    [SerializeField]
    Transform to;

    [SerializeField]
    LayerMask layerMask;

    [SerializeField]
    Vector2 windowCoord;

    [SerializeField]
    Vector2 desktopCoord;

    void Update()
    {
        var from2to = to.position - from.position;
        var dir = from2to.normalized;
        var distance = from2to.magnitude;
        var result = UwcWindowTexture.RayCast(from.position, dir, distance, layerMask);
        if (result.hit) {
            Debug.DrawLine(from.position, to.position, Color.red);
            Debug.DrawRay(result.position, result.normal, Color.green);
            windowCoord = result.windowCoord;
            desktopCoord = result.desktopCoord;
        } else {
            Debug.DrawLine(from.position, to.position, Color.yellow);
            windowCoord = new Vector2(-1, -1);
            desktopCoord = new Vector2(-1, -1);
        }
    }
}

}