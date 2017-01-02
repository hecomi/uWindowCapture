using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowManager))]
public abstract class UwcLayouter : MonoBehaviour
{
    UwcWindowManager manager_;

    void Start()
    {
        manager_ = GetComponent<UwcWindowManager>();
    }

    void LateUpdate()
    {
        Layout(manager_.windows);
    }

    public abstract void Layout(Dictionary<System.IntPtr, UwcWindowObject> windows);
}

}