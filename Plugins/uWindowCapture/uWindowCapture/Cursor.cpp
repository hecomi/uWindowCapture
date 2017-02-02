#include "Cursor.h"
#include "Debug.h"
#include "Util.h"
#include "WindowManager.h"
#include "Unity.h"
#include "Message.h"

using namespace Microsoft::WRL;



Cursor::Cursor()
{
    // StartCapture();
}


Cursor::~Cursor()
{
    StopCapture();
    DeleteBitmap();
}


void Cursor::StartCapture()
{
    threadLoop_.Start([&] 
    {
        Capture();
    }, std::chrono::microseconds(16666));
}


void Cursor::StopCapture()
{
    threadLoop_.Stop();
}


UINT Cursor::GetX() const
{
    return x_;
}


UINT Cursor::GetY() const
{
    return y_;
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
        OutputApiError(__FUNCTION__, "GetCursorInfo");
        return false;
    }

    x_ = cursorInfo.ptScreenPos.x;
    y_ = cursorInfo.ptScreenPos.y;

    /*
    if (cursorInfo.flags != CURSOR_SHOWING)
    {
        buffer_.Reset();
        hasCaptured_ = true;
        return true;
    }
    */

    ICONINFO iconInfo;
    ::ZeroMemory(&iconInfo, sizeof(ICONINFO));
    if (!::GetIconInfo(cursorInfo.hCursor, &iconInfo))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return false;
    }

    BITMAP bmpColor;
    ::ZeroMemory(&bmpColor, sizeof(BITMAP));
    if (!::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpColor))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return false;
    }
    ScopedReleaser hbmColorReleaser([&] { ::DeleteObject(iconInfo.hbmColor); });

    auto desktopDc = ::GetDC(GetDesktopWindow());
    ScopedReleaser hDcReleaser([&] { ::DeleteDC(desktopDc); });

    auto hDcMem = ::CreateCompatibleDC(NULL);
    ScopedReleaser hDcMemReleaser([&] { ::DeleteDC(hDcMem); });

    CreateBitmapIfNeeded(desktopDc, bmpColor.bmWidth, bmpColor.bmHeight);

    BITMAPINFOHEADER bmi {};
    bmi.biWidth       = static_cast<LONG>(width_);
    bmi.biHeight      = -static_cast<LONG>(height_);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    Buffer<BYTE> desktop(width_ * height_ * 4);
    Buffer<BYTE> desktopWithIcon(width_ * height_ * 4);
    Buffer<BYTE> icon(width_ * height_ * 4);

    HGDIOBJ preObject = ::SelectObject(hDcMem, bitmap_);
    {
        // BitBlt desktop image
        ::BitBlt(hDcMem, 0, 0, width_, height_, desktopDc, x_, y_, SRCCOPY);

        if (!::GetDIBits(hDcMem, bitmap_, 0, height_, desktop.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError(__FUNCTION__, "GetDIBits");
        }

        // Draw icon
        if (!::DrawIcon(hDcMem, 0, 0, cursorInfo.hCursor))
        {
            OutputApiError(__FUNCTION__, "DrawIcon");
        }

        if (!::GetDIBits(hDcMem, bitmap_, 0, height_, desktopWithIcon.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError(__FUNCTION__, "GetDIBits");
        }

        // Icon only
        if (!::GetDIBits(hDcMem, iconInfo.hbmColor, 0, height_, icon.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError(__FUNCTION__, "GetDIBits");
        }
    }
    ::SelectObject(hDcMem, preObject);

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);

        auto buffer32 = buffer_.As<UINT>();
        const auto desktop32 = desktop.As<UINT>();
        const auto desktopWithIcon32 = desktopWithIcon.As<UINT>();
        const auto icon32 = icon.As<UINT>();

        const auto n = width_ * height_;
        for (UINT x = 0; x < width_; ++x) 
        {
            for (UINT y = 0; y < height_; ++y)
            {
                const auto i = y * width_ + x;
                const auto j = (height_ - 1 - y) * width_ + x;

                if (icon[4 * j + 3] > 0)
                {
                    buffer32[i] = icon32[j];
                    buffer_[4 * i + 3] = 255;
                }
                else
                {
                    buffer_[4 * i + 0] = desktopWithIcon[4 * j + 0];
                    buffer_[4 * i + 1] = desktopWithIcon[4 * j + 1];
                    buffer_[4 * i + 2] = desktopWithIcon[4 * j + 2];

                    desktop[4 * j + 3] = desktopWithIcon[4 * j + 3] = 0;
                    buffer_[4 * i + 3] = (desktop32[j] != desktopWithIcon32[j]) ? 255 : 0;
                }
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

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

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


void Cursor::CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height)
{
    std::lock_guard<std::mutex> lock(bufferMutex_);

    if (width_ == width && height_ == height) return;

    width_ = width;
    height_ = height;
    buffer_.ExpandIfNeeded(width * height * 4);

    DeleteBitmap();
    bitmap_ = ::CreateCompatibleBitmap(hDc, width, height);
}


void Cursor::DeleteBitmap()
{
    if (bitmap_ != nullptr) 
    {
        if (!::DeleteObject(bitmap_)) OutputApiError(__FUNCTION__, "DeleteObject");
        bitmap_ = nullptr;
    }
}