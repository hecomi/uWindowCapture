using UnityEngine;
using UnityEngine.EventSystems;

namespace uWindowCapture
{

[RequireComponent(typeof(UwcWindowListItem))]
public class UwcWindowListItemMouseEvents 
    : MonoBehaviour
    , IPointerClickHandler
{
    UwcWindowListItem item_;

    void Awake()
    {
        item_ = GetComponent<UwcWindowListItem>();
    }

    public void OnPointerClick(PointerEventData eventData)
    {
    }
}

}