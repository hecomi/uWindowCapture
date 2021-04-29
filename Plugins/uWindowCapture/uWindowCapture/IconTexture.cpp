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
    if (isExtracted_ && hIcon_)
    {
        ::DestroyIcon(hIcon_);
    }
}


void IconTexture::InitIcon()
{
    InitIconHandle();

    if (!hIcon_) return;

    ICONINFO info;
    if (!::GetIconInfo(hIcon_, &info))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return;
    }
    ScopedReleaser iconReleaser([&] 
    { 
        ::DeleteObject(info.hbmColor); 
        ::DeleteObject(info.hbmMask); 
    });

    BITMAP bmpColor { 0 };
    ::GetObject(info.hbmColor, sizeof(bmpColor), &bmpColor);
    width_ = bmpColor.bmWidth;
    height_ = bmpColor.bmHeight;
}


void IconTexture::InitIconHandle()
{
    // Now we cannot get an icon from UWP app from these methods.

    const auto hWnd = window_->GetWindowHandle();

    if (window_->IsAltTab())
    {
        hIcon_ = reinterpret_cast<HICON>(::GetClassLongPtr(hWnd, GCLP_HICON));
        if (hIcon_) return;

        constexpr UINT timeout = 100;
        auto lr = ::SendMessageTimeoutW(
            hWnd, 
            WM_GETICON, 
            ICON_BIG, 
            0, 
            SMTO_ABORTIFHUNG | SMTO_BLOCK, 
            timeout, 
            reinterpret_cast<PDWORD_PTR>(&hIcon_));
        if (hIcon_ && SUCCEEDED(lr)) return;
    }

    hIcon_ = ::LoadIcon(0, IDI_APPLICATION);
    if (hIcon_) return;

    Debug::Error(__FUNCTION__, " => Could not get HICON.");
    return;
}


UINT IconTexture::GetWidth() const
{
    return width_ > 0 ? width_ : ::GetSystemMetrics(SM_CXICON);
}


UINT IconTexture::GetHeight() const
{
    return height_ > 0 ? height_ : ::GetSystemMetrics(SM_CYICON);
}


void IconTexture::SetUnityTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* IconTexture::GetUnityTexturePtr() const
{
    return unityTexture_;
}


bool IconTexture::Capture()
{
    if (!hIcon_) return false;

    ICONINFO info;
    if (!::GetIconInfo(hIcon_, &info))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return false;
    }
    ScopedReleaser iconReleaser([&] 
    { 
        ::DeleteObject(info.hbmColor); 
        ::DeleteObject(info.hbmMask); 
    });

    auto hDcMem = ::CreateCompatibleDC(NULL);
    ScopedReleaser hDcReleaser([&] { ::DeleteDC(hDcMem); });

    BITMAPINFOHEADER bmi {};
    bmi.biWidth       = static_cast<LONG>(width_);
    bmi.biHeight      = -static_cast<LONG>(height_);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    Buffer<BYTE> color;
    color.ExpandIfNeeded(width_ * height_ * 4);
    if (!::GetDIBits(hDcMem, info.hbmColor, 0, height_, color.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError(__FUNCTION__, "GetDIBits");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        buffer_.ExpandIfNeeded(width_ * height_ * 4);

        auto* buffer32 = buffer_.As<UINT>();
        const auto* color32 = color.As<UINT>();
        bool areAllPixelsAlphaZero = true;

        const auto n = width_ * height_;
        for (UINT x = 0; x < width_; ++x) 
        {
            for (UINT y = 0; y < height_; ++y)
            {
                const auto i = y * width_ + x;
                const auto j = (height_ - 1 - y) * width_ + x;
                buffer32[j] = color32[i];
                if ((color32[i] & 0xff000000) != 0) areAllPixelsAlphaZero = false;
            }
        }

        if (areAllPixelsAlphaZero)
        {
            for (UINT i = 0; i < n; ++i)
            {
                buffer32[i] |= 0xff000000;
            }
        }
    }

    hasCaptured_ = true;

    return true;
}


bool IconTexture::CaptureOnce()
{
    if (hasCaptured_) return false;

    return Capture();
}


bool IconTexture::Upload()
{
    if (!unityTexture_.load() || buffer_.Empty()) return false;

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    {
        D3D11_TEXTURE2D_DESC desc;
        unityTexture_.load()->GetDesc(&desc);
        if (desc.Width != GetWidth() || desc.Height != GetHeight())
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


bool IconTexture::UploadOnce()
{
    if (hasUploaded_) return false;

    return Upload();
}


bool IconTexture::Render()
{
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

    MessageManager::Get().Add({ MessageType::IconCaptured, window_->GetId(), window_->GetWindowHandle() });

    return true;
}


bool IconTexture::RenderOnce()
{
    if (hasRendered_) return true;

    return Render();
}