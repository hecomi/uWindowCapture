#include <dwmapi.h>
#include "WindowTexture.h"
#include "Window.h"
#include "WindowManager.h"
#include "UploadManager.h"
#include "Message.h"
#include "Unity.h"
#include "Debug.h"
#include "Util.h"

using namespace Microsoft::WRL;



WindowTexture::WindowTexture(Window* window)
    : window_(window)
{
}


WindowTexture::~WindowTexture()
{
    std::lock_guard<std::mutex> lock(bufferMutex_);
    DeleteBitmap();
}


void WindowTexture::SetUnityTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* WindowTexture::GetUnityTexturePtr() const
{
    return unityTexture_;
}


void WindowTexture::SetCaptureMode(CaptureMode mode)
{
    captureMode_ = mode;
}


CaptureMode WindowTexture::GetCaptureMode() const
{
    return captureMode_;
}


UINT WindowTexture::GetWidth() const
{
    return textureWidth_;
}


UINT WindowTexture::GetHeight() const
{
    return textureHeight_;
}


void WindowTexture::CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height)
{
    std::lock_guard<std::mutex> lock(bufferMutex_);

    if (bufferWidth_ == width && bufferHeight_ == height) return;
    if (width == 0 || height == 0) return;

    bufferWidth_ = width;
    bufferHeight_ = height;
    buffer_.ExpandIfNeeded(width * height * 4);

    DeleteBitmap();
    bitmap_ = ::CreateCompatibleBitmap(hDc, width, height);

    MessageManager::Get().Add({ MessageType::WindowSizeChanged, window_->GetId(), window_->GetHandle() });
}


void WindowTexture::DeleteBitmap()
{
    if (bitmap_ != nullptr) 
    {
        if (!::DeleteObject(bitmap_)) OutputApiError(__FUNCTION__, "DeleteObject");
        bitmap_ = nullptr;
    }
}


bool WindowTexture::Capture()
{
    auto hWnd = window_->GetHandle();

    auto hDc = ::GetDC(hWnd);
    ScopedReleaser hDcReleaser([&] { ::ReleaseDC(hWnd, hDc); });

    BITMAP bmpHeader;
    ZeroMemory(&bmpHeader, sizeof(BITMAP));
    auto hBitmap = ::GetCurrentObject(hDc, OBJ_BITMAP);
    GetObject(hBitmap, sizeof(BITMAP), &bmpHeader);
    auto dcWidth = bmpHeader.bmWidth;
    auto dcHeight = bmpHeader.bmHeight;

    // If failed, use window size (for example, UWP uses this)
    if (dcWidth == 0 || dcHeight == 0)
    {
        dcWidth = window_->GetWidth();
        dcHeight = window_->GetHeight();
    }

    if (dcWidth == 0 || dcHeight == 0)
    {
        return false;
    }

    // DPI scale
    float dpiScaleX = std::fmax(static_cast<float>(window_->GetWidth()) / dcWidth, 1.f);
    float dpiScaleY = std::fmax(static_cast<float>(window_->GetHeight()) / dcHeight, 1.f);

    if (captureMode_ == CaptureMode::BitBlt)
    {
        // Remove frame areas
        const auto frameWidth = window_->GetWidth() - window_->GetClientWidth();
        const auto frameHeight = window_->GetHeight() - window_->GetClientHeight();

        dcWidth -= static_cast<LONG>(ceil(frameWidth / dpiScaleX));
        dcHeight -= static_cast<LONG>(ceil(frameHeight / dpiScaleY));
    }

    CreateBitmapIfNeeded(hDc, dcWidth, dcHeight);

    auto hDcMem = ::CreateCompatibleDC(hDc);
    ScopedReleaser hDcMemRelaser([&] { ::DeleteDC(hDcMem); });

    HGDIOBJ preObject = ::SelectObject(hDcMem, bitmap_);
    ScopedReleaser selectObject([&] { ::SelectObject(hDcMem, preObject); });

    int offsetLeft = 0, offsetRight = 0, offsetTop = 0, offsetBottom = 0;

    switch (captureMode_)
    {
        case CaptureMode::PrintWindow:
        {
            if (!::PrintWindow(hWnd, hDcMem, PW_RENDERFULLCONTENT)) 
            {
                OutputApiError(__FUNCTION__, "PrintWindow");
                return false;
            }
            break;
        }
        case CaptureMode::BitBlt:
        {
            if (!::BitBlt(hDcMem, 0, 0, bufferWidth_, bufferHeight_, hDc, 0, 0, SRCCOPY | CAPTUREBLT)) 
            {
                OutputApiError(__FUNCTION__, "BitBlt");
                return false;
            }
            break;
        }
        default:
        {
            return true;
        }
    }

    // Draw cursor
    const auto cursorWindow = WindowManager::Get().GetCursorWindow();
    if (cursorWindow && cursorWindow->GetHandle() == window_->GetHandle())
    {
        CURSORINFO cursorInfo;
        cursorInfo.cbSize = sizeof(CURSORINFO);
        if (::GetCursorInfo(&cursorInfo))
        {
            if (cursorInfo.flags == CURSOR_SHOWING)
            {
                const auto windowLocalCursorX = static_cast<int>((cursorInfo.ptScreenPos.x - window_->GetX()) / dpiScaleX);
                const auto windowLocalCursorY = static_cast<int>((cursorInfo.ptScreenPos.y - window_->GetY()) / dpiScaleY);
                ::DrawIcon(hDcMem, windowLocalCursorX, windowLocalCursorY, cursorInfo.hCursor);
            }
        }
        else
        {
            OutputApiError(__FUNCTION__, "GetCursorInfo");
        }
    }

    BITMAPINFOHEADER bmi {};
    bmi.biWidth       = static_cast<LONG>(bufferWidth_);
    bmi.biHeight      = -static_cast<LONG>(bufferHeight_);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);

        if (!::GetDIBits(hDcMem, bitmap_, 0, bufferHeight_, buffer_.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
        {
            OutputApiError(__FUNCTION__, "GetDIBits");
            return false;
        }

        // Remove dropshadow area
        if (captureMode_ == CaptureMode::PrintWindow)
        {
            RECT dwmRect;
            ::DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &dwmRect, sizeof(RECT));

            RECT windowRect;
            ::GetWindowRect(hWnd, &windowRect);

            offsetX_ = dwmRect.left - windowRect.left;
            offsetY_ = dwmRect.top - windowRect.top;
            textureWidth_ = dwmRect.right - dwmRect.left;
            textureHeight_ = dwmRect.bottom - dwmRect.top;
        }
        else
        {
            offsetX_ = 0;
            offsetY_ = 0;
            textureWidth_ = bufferWidth_.load();
            textureHeight_ = bufferHeight_.load();
        }
    }

    return true;
}


bool WindowTexture::Upload()
{
    if (!unityTexture_.load()) return false;

    UWC_SCOPE_TIMER(UploadTexture)

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

    bool shouldUpdateTexture = true;

    if (sharedTexture_)
    {
        D3D11_TEXTURE2D_DESC desc;
        sharedTexture_->GetDesc(&desc);
        if (desc.Width == GetWidth() && desc.Height == GetHeight())
        {
            shouldUpdateTexture = false;
        }
    }

    auto& uploader = WindowManager::GetUploadManager();
    if (!uploader) return false;

    if (shouldUpdateTexture)
    {
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
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);

        const UINT rawPitch = bufferWidth_ * 4;
        const auto *start = &buffer_[offsetX_ * 4 + offsetY_ * rawPitch];

        ComPtr<ID3D11DeviceContext> context;
        uploader->GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(sharedTexture_.Get(), 0, nullptr, start, rawPitch, 0);
        context->Flush();
    }

    return true;
}


bool WindowTexture::Render()
{
    if (!unityTexture_.load() || !sharedTexture_ || !sharedHandle_) return false;

    UWC_SCOPE_TIMER(Render)

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

    MessageManager::Get().Add({ MessageType::WindowCaptured, window_->GetId(), window_->GetHandle() });

    return true;
}