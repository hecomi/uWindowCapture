Shader "uWindowCapture/Transparent"
{

Properties
{
    _Color("Color", Color) = (1, 1, 1, 1)
    _MainTex("Texture", 2D) = "white" {}
    [Enum(UnityEngine.Rendering.CullMode)] _Cull("Culling", Int) = 2
    [Enum(UnityEngine.Rendering.BlendMode)] _BlendSrc("Blend Src", Float) = 5 
    [Enum(UnityEngine.Rendering.BlendMode)] _BlendDst("Blend Dst", Float) = 10
    [Toggle][KeyEnum(Off, On)] _ZWrite("ZWrite", Float) = 1
    [Toggle(UWC_FLIP_X)] _FlipX("Flip X", Int) = 0
    [Toggle(UWC_FLIP_Y)] _FlipY("Flip Y", Int) = 0
}

SubShader
{
    Tags 
    { 
        "Queue" = "Transparent"
        "RenderType" = "Transparent" 
        "IgnoreProjector" = "True" 
        "PreviewType" = "Plane"
    }

    Pass
    {
        Cull [_Cull]
        Blend [_BlendSrc] [_BlendDst]
        ZWrite [_ZWrite]

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

Fallback "Unlit/Transparent"

}