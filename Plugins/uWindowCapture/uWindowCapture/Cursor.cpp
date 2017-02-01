#include "Cursor.h"
#include "Debug.h"
#include "Util.h"
#include "WindowManager.h"
#include "Unity.h"
#include "Message.h"

using namespace Microsoft::WRL;



Cursor::Cursor()
{
    threadLoop_.Start([&] 
    {
        Capture();
    }, std::chrono::microseconds(16666));
}


Cursor::~Cursor()
{
}


UINT Cursor::GetWidth() const
{
    return width_;
}


UINT Cursor::GetHeight() const
{
    return height_;
}


bool Cursor::HasCaptured() const
{
    return hasCaptured_;
}


bool Cursor::HasUploaded() const
{
    return hasUploaded_;
}


void Cursor::SetUnityTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* Cursor::GetUnityTexturePtr() const
{
    return unityTexture_;
}


bool Cursor::Capture()
{
    std::lock_guard<std::mutex> lock(cursorMutex_);

    CURSORINFO cursorInfo;
    cursorInfo.cbSize = sizeof(CURSORINFO);
    if (!::GetCursorInfo(&cursorInfo))
    {
        OutputApiError("GetCursorInfo");
        return false;
    }

    x_ = cursorInfo.ptScreenPos.x;
    y_ = cursorInfo.ptScreenPos.y;

    if (cursorInfo.flags != CURSOR_SHOWING)
    {
        buffer_.Reset();
        hasCaptured_ = true;
        return true;
    }

    ICONINFO iconInfo;
    ZeroMemory(&iconInfo, sizeof(ICONINFO));
    if (!::GetIconInfo(cursorInfo.hCursor, &iconInfo))
    {
        OutputApiError("GetIconInfo");
        return false;
    }

    BITMAP bmpColor;
    ZeroMemory(&bmpColor, sizeof(BITMAP));
    if (!::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpColor))
    {
        OutputApiError("GetIconInfo");
        return false;
    }
    ScopedReleaser hbmColorReleaser([&] { ::DeleteObject(iconInfo.hbmColor); });

    BITMAP bmpMask;
    ZeroMemory(&bmpMask, sizeof(BITMAP));
    if (!::GetObject(iconInfo.hbmMask, sizeof(BITMAP), &bmpMask))
    {
        OutputApiError("GetIconInfo");
        return false;
    }
    ScopedReleaser hbmMaskReleaser([&] { ::DeleteObject(iconInfo.hbmMask); });

    width_ = bmpColor.bmWidth;
    height_ = bmpColor.bmHeight;

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

    // Get color image
    Buffer<BYTE> color;
    color.ExpandIfNeeded(width_ * height_ * 4);
    if (!::GetDIBits(hDcMem, iconInfo.hbmColor, 0, height_, color.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError("GetDIBits");
        return false;
    }
    
    // Get mask image
    Buffer<BYTE> mask;
    mask.ExpandIfNeeded(width_ * height_ * 4);
    if (!::GetDIBits(hDcMem, iconInfo.hbmMask, 0, height_ * 2, mask.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError("GetDIBits");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        buffer_.ExpandIfNeeded(width_ * height_ * 4);

        auto buffer32 = buffer_.As<UINT>();
        auto color32 = color.As<UINT>();
        auto mask32 = mask.As<UINT>();

        const auto n = width_ * height_;
        for (UINT x = 0; x < width_; ++x) 
        {
            for (UINT y = 0; y < height_; ++y)
            {
                const auto i = y * width_ + x;
                const auto j = (height_ - 1 - y) * width_ + x;
                buffer32[j] = color32[i];
            }
        }
    }

    hasCaptured_ = true;

    return true;
}


bool Cursor::Upload()
{
    if (!hasCaptured_) return false;

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

    hasCaptured_ = false;
    hasUploaded_ = true;

    return true;
}


bool Cursor::Render()
{
    if (!hasCaptured_) return false;

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

    MessageManager::Get().Add({ MessageType::CursorCaptured, -1, nullptr });

    hasUploaded_ = false;

    return true;
}