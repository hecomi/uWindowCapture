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
    return bufferWidth_;
}


UINT WindowTexture::GetHeight() const
{
    return bufferHeight_;
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
        if (!::DeleteObject(bitmap_)) OutputApiError("DeleteObject");
        bitmap_ = nullptr;
    }
}


bool WindowTexture::Capture()
{
    auto hWnd = window_->GetHandle();
    auto hDc = ::GetDC(hWnd);

    {
        // Check window size from HDC to get correct values for non-DPI-scaled applications.
        BITMAP header;
        ZeroMemory(&header, sizeof(BITMAP));
        auto hBitmap = GetCurrentObject(hDc, OBJ_BITMAP);
        GetObject(hBitmap, sizeof(BITMAP), &header);
        auto width = header.bmWidth;
        auto height = header.bmHeight;

        // If failed, use window size (for example, UWP uses this)
        if (width == 0 || height == 0)
        {
            width = window_->GetWidth();
            height = window_->GetHeight();
        }

        if (width == 0 || height == 0)
        {
            if (!::ReleaseDC(hWnd, hDc)) OutputApiError("ReleaseDC");
            return false;
        }

        CreateBitmapIfNeeded(hDc, width, height);
    }

    auto hDcMem = ::CreateCompatibleDC(hDc);
    HGDIOBJ preObject = ::SelectObject(hDcMem, bitmap_);

    BOOL result = false;
    switch (captureMode_)
    {
        case CaptureMode::PrintWindow:
        {
            result = ::PrintWindow(hWnd, hDcMem, PW_RENDERFULLCONTENT);
            if (!result) OutputApiError("PrintWindow");
            break;
        }
        case CaptureMode::BitBlt:
        {
            result = ::BitBlt(hDcMem, 0, 0, bufferWidth_, bufferHeight_, hDc, 0, 0, SRCCOPY);
            if (!result) OutputApiError("BitBlt");
            break;
        }
        case CaptureMode::BitBltAlpha:
        {
            result = ::BitBlt(hDcMem, 0, 0, bufferWidth_, bufferHeight_, hDc, 0, 0, SRCCOPY | CAPTUREBLT);
            if (!result) OutputApiError("BitBlt");
            break;
        }
        default:
        {
            break;
        }
    }

    if (result)
    {
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
                OutputApiError("GetDIBits");
            }
        }
    }

    ::SelectObject(hDcMem, preObject);

    if (!::DeleteDC(hDcMem)) OutputApiError("DeleteDC");
    if (!::ReleaseDC(hWnd, hDc)) OutputApiError("ReleaseDC");

    return !FAILED(result);
}


bool WindowTexture::Upload()
{
    if (!unityTexture_.load()) return false;

    UWC_SCOPE_TIMER(UploadTexture)

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    {
        D3D11_TEXTURE2D_DESC desc;
        unityTexture_.load()->GetDesc(&desc);
        if (desc.Width != bufferWidth_ && desc.Height != bufferHeight_)
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
        if (desc.Width == bufferWidth_ && desc.Height == bufferHeight_)
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
        ComPtr<ID3D11DeviceContext> context;
        uploader->GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(sharedTexture_.Get(), 0, nullptr, buffer_.Get(), bufferWidth_ * 4, 0);
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