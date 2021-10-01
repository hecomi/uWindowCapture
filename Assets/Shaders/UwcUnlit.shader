Shader "uWindowCapture/Unlit"
{

Properties
{
    _Color("Color", Color) = (1, 1, 1, 1)
    _MainTex("Texture", 2D) = "white" {}
    [Enum(UnityEngine.Rendering.CullMode)] _Cull("Culling", Int) = 2
    [Toggle(UWC_FLIP_X)] _FlipX("Flip X", Int) = 0
    [Toggle(UWC_FLIP_Y)] _FlipY("Flip Y", Int) = 0
}

SubShader
{
    Tags 
    { 
        "RenderType" = "Opaque" 
        "Queue" = "Geometry"
        "PreviewType" = "Plane"
    }

    Pass
    {
        Cull [_Cull]

        CGPROGRAM
        #include "./UwcCommon.cginc"
        #pragma vertex vert
        #pragma fragment frag
        #pragma multi_compile_fog
        #pragma multi_compile ___ UWC_FLIP_X
        #pragma multi_compile ___ UWC_FLIP_Y
        ENDCG
    }
}

Fallback "Unlit/Texture"

}