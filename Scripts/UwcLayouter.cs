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
        UpdateLayout(manager_.windows);
    }

    public virtual void InitWindow(UwcWindowObject window)
    {
    }

    public virtual void UpdateLayout(Dictionary<System.IntPtr, UwcWindowObject> windows)
    {
    }
}

}