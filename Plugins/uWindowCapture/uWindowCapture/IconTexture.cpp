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
    if (hasCaptured_) return false;

    auto hWnd = window_->GetWindowHandle();

    // TODO: cannot get icon when the window is UWP.
    auto hIcon = reinterpret_cast<HICON>(::GetClassLongPtr(hWnd, GCLP_HICON));
    if (hIcon == nullptr)
    {
        const UINT timeout = 100;
        const auto hr = ::SendMessageTimeout(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, timeout, reinterpret_cast<PDWORD_PTR>(&hIcon));
        if (FAILED(hr))
        {
            hIcon = reinterpret_cast<HICON>(::LoadImage(window_->GetInstance(), IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED));
            if (hIcon == nullptr)
            {
                Debug::Error(__FUNCTION__, " => Could not get HICON.");
                return false;
            }
        }
    }

    const auto width = GetWidth();
    const auto height = GetHeight();

    ICONINFO info;
    if (!::GetIconInfo(hIcon, &info))
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
    bmi.biWidth       = static_cast<LONG>(width);
    bmi.biHeight      = -static_cast<LONG>(height);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    // Get color image
    Buffer<BYTE> color;
    color.ExpandIfNeeded(width * height * 4);
    if (!::GetDIBits(hDcMem, info.hbmColor, 0, height, color.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError(__FUNCTION__, "GetDIBits");
        return false;
    }
    
    // Get mask image
    Buffer<BYTE> mask;
    mask.ExpandIfNeeded(width * height * 4);
    if (!::GetDIBits(hDcMem, info.hbmMask, 0, height, mask.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError(__FUNCTION__, "GetDIBits");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        buffer_.ExpandIfNeeded(width * height * 4);

        auto buffer32 = buffer_.As<UINT>();
        auto color32 = color.As<UINT>();
        auto mask32 = mask.As<UINT>();

        const auto n = width * height;
        for (UINT x = 0; x < width; ++x) 
        {
            for (UINT y = 0; y < height; ++y)
            {
                const auto i = y * width + x;
                const auto j = (height - 1 - y) * width + x;
                buffer32[j] = color32[i] ^ mask32[i];
            }
        }
    }

    hasCaptured_ = true;

    return true;
}


bool IconTexture::UploadOnce()
{
    if (hasUploaded_) return false;

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

    MessageManager::Get().Add({ MessageType::IconCaptured, window_->GetId(), window_->GetWindowHandle() });

    return true;
}