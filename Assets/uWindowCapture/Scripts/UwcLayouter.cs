using UnityEngine;
using System.Collections.Generic;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowObjectManager))]
public abstract class UwcLayouter : MonoBehaviour
{
    UwcWindowObjectManager manager_;

    void Start()
    {
        manager_ = GetComponent<UwcWindowObjectManager>();
    }

    void LateUpdate()
    {
        UpdateLayout(manager_.windows);
    }

    public virtual void InitWindow(UwcWindowObject window)
    {
    }

    public virtual void UpdateLayout(Dictionary<int, UwcWindowObject> windows)
    {
    }
}

}