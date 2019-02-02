using UnityEngine;
using UnityEditor;

namespace uWindowCapture
{

[CustomEditor(typeof(UwcManager))]
public class UwcManagerEditor : Editor
{
    UwcManager manager
    {
        get { return target as UwcManager; }
    }

    SerializedProperty windowTitlesUpdateTiming;

    void OnEnable()
    {
        windowTitlesUpdateTiming = serializedObject.FindProperty("windowTitlesUpdateTiming");
    }

    public override void OnInspectorGUI()
    {
        serializedObject.Update();
        Draw();
        serializedObject.ApplyModifiedProperties();
    }

    void Draw()
    {
        var debugMode = (DebugMode)EditorGUILayout.EnumPopup("Debug Mode", manager.debugModeFromInspector);
        if (debugMode != manager.debugModeFromInspector)
        {
            manager.debugModeFromInspector = debugMode;
        }

        EditorGUILayout.PropertyField(windowTitlesUpdateTiming);
    }
}

}