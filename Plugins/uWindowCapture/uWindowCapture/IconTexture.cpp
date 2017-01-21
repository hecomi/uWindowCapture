#include "IconTexture.h"
#include "Window.h"
#include "WindowManager.h"
#include "UploadManager.h"
#include "Debug.h"
#include "Unity.h"
#include "Util.h"
#include "Message.h"

using namespace Microsoft::WRL;



IconTexture::IconTexture(Window* window)
    : window_(window)
{
}


IconTexture::~IconTexture()
{
}


UINT IconTexture::GetWidth() const
{
    return ::GetSystemMetrics(SM_CXICON);
}


UINT IconTexture::GetHeight() const
{
    return ::GetSystemMetrics(SM_CYICON);
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
    if (hasCaptured_) return true;

    auto hWnd = window_->GetHandle();


    auto hIcon = reinterpret_cast<HICON>(GetClassLongPtr(hWnd, GCLP_HICON));
    if (hIcon == nullptr)
    {
        hIcon = reinterpret_cast<HICON>(SendMessage(hWnd, WM_GETICON, ICON_BIG, 0));
        if (hIcon == nullptr)
        {
            Debug::Error(__FUNCTION__, " => Could not get HICON.");
            return false;
        }
    }

    const auto width = GetWidth();
    const auto height = GetHeight();

    ICONINFO info;
    if (!::GetIconInfo(hIcon, &info))
    {
        OutputApiError("GetIconInfo");
        return false;
    }

    BITMAPINFOHEADER bmi {};
    bmi.biWidth       = static_cast<LONG>(width);
    bmi.biHeight      = -static_cast<LONG>(height);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    Buffer<BYTE> mask;
    mask.ExpandIfNeeded(width * height * 4);

    auto hDcMem = ::CreateCompatibleDC(NULL);
    ScopedReleaser hDcReleaser([&] { ::DeleteDC(hDcMem); });

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);

        buffer_.ExpandIfNeeded(width * height * 4);
        if (!::GetDIBits(hDcMem, info.hbmColor, 0, height, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError("GetDIBits");
            return false;
        }

        if (!::GetDIBits(hDcMem, info.hbmMask, 0, height, mask.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError("GetDIBits");
            return false;
        }

        auto color32 = buffer_.As<UINT>();
        auto mask32 = mask.As<UINT>();
        for (UINT i = 0; i < width * height; ++i)
        {
            color32[i] ^= mask32[i];
        }
    }

    hasCaptured_ = true;

    return true;
}


bool IconTexture::UploadOnce()
{
    if (hasUploaded_) return true;

    if (!unityTexture_.load() || buffer_.Empty()) return false;

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    {
        D3D11_TEXTURE2D_DESC desc;
        unityTexture_.load()->GetDesc(&desc);
        if (desc.Width != GetWidth() && desc.Height != GetHeight())
        {
            Debug::Error(__FUNCTION__, " => Texture size is wrong.");
            return false;
        }
    }

    auto& uploader = WindowManager::GetUploadManager();
    if (!uploader) return false;

    sharedTexture_ = uploader->CreateCompatibleSharedTexture(unityTexture_.load());
    if (!sharedTexture_)
    {
        Debug::Error(__FUNCTION__, " => Shared texture is null.");
        return false;
    }

    ComPtr<IDXGIResource> dxgiResource;
    sharedTexture_.As(&dxgiResource);
    if (FAILED(dxgiResource->GetSharedHandle(&sharedHandle_)))
    {
        Debug::Error(__FUNCTION__, " => GetSharedHandle() failed.");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        ComPtr<ID3D11DeviceContext> context;
        uploader->GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(sharedTexture_.Get(), 0, nullptr, buffer_.Get(), GetWidth() * 4, 0);
        context->Flush();
    }

    hasUploaded_ = true;

    return true;
}


bool IconTexture::RenderOnce()
{
    if (hasRendered_) return true;

    if (!unityTexture_.load() || !sharedTexture_ || !sharedHandle_) return false;

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    ComPtr<ID3D11DeviceContext> context;
    GetUnityDevice()->GetImmediateContext(&context);

    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(GetUnityDevice()->OpenSharedResource(sharedHandle_, __uuidof(ID3D11Texture2D), &texture)))
    {
        Debug::Error(__FUNCTION__, " => OpenSharedResource() failed.");
        return false;
    }

    context->CopyResource(unityTexture_.load(), texture.Get());

    MessageManager::Get().Add({ MessageType::IconCaptured, window_->GetId(), window_->GetHandle() });

    return true;
}