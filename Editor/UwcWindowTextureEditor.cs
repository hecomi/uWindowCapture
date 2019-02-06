using UnityEngine;
using UnityEditor;

namespace uWindowCapture
{

[CustomEditor(typeof(UwcWindowTexture))]
public class UwcWindowTextureEditor : Editor
{
    UwcWindowTexture texture
    {
        get { return target as UwcWindowTexture; }
    }

    UwcWindow window
    {
        get { return texture.window; }
    }

    string error_;
    string error 
    { 
        get
        {
            return error_;
        }
        set
        {
            if (string.IsNullOrEmpty(value)) {
                error_ = "";
            } else {
                error_ = string.IsNullOrEmpty(error_) ? value : (error_ + "\n" + value);
            }
        }
    }

    bool targetFold_ = true;
    bool captureSettingFold_ = true;
    bool scaleSettingFold_ = true;
    bool windowInformationFold_ = true;

    SerializedProperty updateTitle;
    SerializedProperty childWindowPrefab;
    SerializedProperty childWindowZDistance;
    SerializedProperty captureMode;
    SerializedProperty capturePriority;
    SerializedProperty captureRequestTiming;
    SerializedProperty captureFrameRate;
    SerializedProperty drawCursor;
    SerializedProperty scaleControlType;
    SerializedProperty scalePer1000Pixel;

    void OnEnable()
    {
        updateTitle = serializedObject.FindProperty("updateTitle");
        childWindowPrefab = serializedObject.FindProperty("childWindowPrefab");
        childWindowZDistance = serializedObject.FindProperty("childWindowZDistance");
        captureMode = serializedObject.FindProperty("captureMode");
        capturePriority = serializedObject.FindProperty("capturePriority");
        captureRequestTiming = serializedObject.FindProperty("captureRequestTiming");
        captureFrameRate = serializedObject.FindProperty("captureFrameRate");
        drawCursor = serializedObject.FindProperty("drawCursor");
        scaleControlType = serializedObject.FindProperty("scaleControlType");
        scalePer1000Pixel = serializedObject.FindProperty("scalePer1000Pixel");
    }

    public override void OnInspectorGUI()
    {
        error = "";

        serializedObject.Update();
        {
            EditorGUILayout.Space();
            EditorUtils.Fold("Target", ref targetFold_, () => { DrawTargetSettings(); });
            EditorUtils.Fold("Capture Settings", ref captureSettingFold_, () => { DrawCaptureSettings(); });
            EditorUtils.Fold("Scale Settings", ref scaleSettingFold_, () => { DrawScaleSettings(); });
            EditorUtils.Fold("Window Information", ref windowInformationFold_, () => { DrawWindowInformation(); });
        }
        serializedObject.ApplyModifiedProperties();

        DrawError();
    }

    void DrawError()
    {
        if (!string.IsNullOrEmpty(error)) {
            EditorGUILayout.HelpBox(error, UnityEditor.MessageType.Error);
        }
    }

    void DrawTargetSettings()
    {
        var type = (WindowTextureType)EditorGUILayout.EnumPopup("Type", texture.type);
        if (type != texture.type) {
            Undo.RecordObject(target, "Inspector");
            texture.type = type;
        }

        var searchTiming = (WindowSearchTiming)EditorGUILayout.EnumPopup("Search Timing", texture.searchTiming);
        if (searchTiming != texture.searchTiming) {
            Undo.RecordObject(target, "Inspector");
            texture.searchTiming = searchTiming;
        }

        switch (type)
        {
            case WindowTextureType.Window:
                var title = EditorGUILayout.TextField("Partial Window Title", texture.partialWindowTitle);
                if (title != texture.partialWindowTitle) {
                    Undo.RecordObject(target, "Inspector");
                    texture.partialWindowTitle = title;
                }
                EditorGUILayout.PropertyField(updateTitle);
                var altTabWindow = EditorGUILayout.Toggle("Alt Tab Window", texture.altTabWindow);
                if (altTabWindow != texture.altTabWindow) {
                    Undo.RecordObject(target, "Inspector");
                    texture.altTabWindow = altTabWindow;
                }
                var createChildWindows = EditorGUILayout.Toggle("Create Child Windows", texture.createChildWindows);
                if (createChildWindows != texture.createChildWindows) {
                    Undo.RecordObject(target, "Inspector");
                    texture.createChildWindows = createChildWindows;
                }
                if (texture.createChildWindows) {
                    EditorGUILayout.PropertyField(childWindowPrefab);
                    EditorGUILayout.PropertyField(childWindowZDistance);
                }
                break;
            case WindowTextureType.Desktop:
                var index = EditorGUILayout.IntField("Desktop Index", texture.desktopIndex);
                if (index != texture.desktopIndex) {
                    Undo.RecordObject(target, "Inspector");
                    texture.desktopIndex = index;
                }
                break;
            case WindowTextureType.Child:
                if (window == null || !window.isChild) {
                    error += "Type: Child should be set only by UwcWindowTextureChildrenManager.";
                }
                break;
        }

        EditorGUILayout.Space();
    }

    void DrawCaptureSettings()
    {
        EditorGUILayout.PropertyField(captureMode);
        EditorGUILayout.PropertyField(capturePriority);
        EditorGUILayout.PropertyField(captureRequestTiming);
        EditorGUILayout.PropertyField(captureFrameRate);
        EditorGUILayout.PropertyField(drawCursor);

        EditorGUILayout.Space();
    }

    void DrawScaleSettings()
    {
        EditorGUILayout.PropertyField(scaleControlType);
        if (texture.scaleControlType == WindowTextureScaleControlType.BaseScale) {
            EditorGUILayout.PropertyField(scalePer1000Pixel);
        }

        EditorGUILayout.Space();
    }

    void DrawWindowInformation()
    {
        if (!Application.isPlaying) {
            EditorGUILayout.HelpBox("Window information will be shown here while playing.", UnityEditor.MessageType.Info);
            return;
        } else if (window == null) {
            EditorGUILayout.HelpBox("Window is not assigned.", UnityEditor.MessageType.Info);
            return;
        }

        EditorGUILayout.IntField("ID", window.id);
        EditorGUILayout.TextField("Window Title", window.title);
        EditorGUILayout.IntField("Window X", window.x);
        EditorGUILayout.IntField("Window Y", window.y);
        EditorGUILayout.IntField("Window Width", window.width);
        EditorGUILayout.IntField("Window Height", window.height);
        EditorGUILayout.IntField("Window Z-Order", window.zOrder);
        EditorGUILayout.Toggle("Alt-Tab Window", window.isAltTabWindow);
        EditorGUILayout.Toggle("Minimized", window.isMinimized);
        EditorGUILayout.Toggle("Maximized", window.isMaximized);

        EditorGUILayout.Space();
    }
}

}