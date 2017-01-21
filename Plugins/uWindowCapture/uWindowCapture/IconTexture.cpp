#include "IconTexture.h"
#include "Window.h"



IconTexture::IconTexture(Window* window)
    : window_(window)
{
}


IconTexture::~IconTexture()
{
}


void IconTexture::SetUnityTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* IconTexture::GetUnityTexturePtr() const
{
    return unityTexture_;
}


bool IconTexture::CaptureOnce()
{
    return true;
}


bool IconTexture::UploadOnce()
{
    return true;
}


bool IconTexture::RenderOnce()
{
    return true;
}