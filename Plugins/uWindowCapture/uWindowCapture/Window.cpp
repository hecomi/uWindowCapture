#include <dwmapi.h>
#include "Window.h"
#include "WindowTexture.h"
#include "IconTexture.h"
#include "WindowManager.h"
#include "Debug.h"
#include "Util.h"



Window::Window(int id, const Data1 &data)
    : id_(id)
    , data1_(data)
{
}


Window::~Window()
{
}


void Window::InitTexture()
{
    windowTexture_ = std::make_shared<WindowTexture>(this);
    iconTexture_ = std::make_shared<IconTexture>(this);
}


void Window::SetData(const Data1& data)
{
    data1_ = data;
}


int Window::GetId() const
{
    return id_;
}


int Window::GetParentId() const
{
    return parentId_;
}


HWND Window::GetWindowHandle() const
{
    return data1_.hWnd;
}


HMONITOR Window::GetMonitorHandle() const
{
    return data1_.hMonitor;
}


HWND Window::GetOwnerHandle() const
{
    return data1_.hOwner;
}


HWND Window::GetParentHandle() const
{
    return data2_.hParent;
}


HINSTANCE Window::GetInstance() const
{
    return data2_.hInstance;
}


DWORD Window::GetProcessId() const
{
    return data2_.processId;
}


DWORD Window::GetThreadId() const
{
    return data2_.threadId;
}


const RECT & Window::GetWindowRect() const
{
    return data1_.windowRect;
}


const RECT & Window::GetClientRect() const
{
    return data1_.clientRect;
}


bool Window::IsAltTab() const
{
    return data2_.isAltTabWindow;
}


bool Window::IsDesktop() const
{
    return data1_.isDesktop;
}


BOOL Window::IsWindow() const
{
    return ::IsWindow(GetWindowHandle());
}


BOOL Window::IsVisible() const
{
    return ::IsWindowVisible(GetWindowHandle());
}


BOOL Window::IsEnabled() const
{
    return ::IsWindowEnabled(GetWindowHandle());
}


BOOL Window::IsUnicode() const
{
    return ::IsWindowUnicode(GetWindowHandle());
}


BOOL Window::IsZoomed() const
{
    return ::IsZoomed(GetWindowHandle());
}


BOOL Window::IsIconic() const
{
    return ::IsIconic(GetWindowHandle());
}


BOOL Window::IsHungUp() const
{
    return ::IsHungAppWindow(GetWindowHandle());
}


BOOL Window::IsTouchable() const
{
    return ::IsTouchWindow(GetWindowHandle(), NULL);
}


BOOL Window::IsApplicationFrameWindow() const
{
    return data2_.isApplicationFrameWindow;
}


BOOL Window::IsUWP() const
{
    return data2_.isUWP;
}


BOOL Window::IsBackground() const
{
    return data2_.isBackground;
}


bool Window::IsWindowsGraphicsCaptureAvailable() const
{
    return windowTexture_ && windowTexture_->IsWindowsGraphicsCaptureAvailable();
}


UINT Window::GetX() const
{
    return data1_.windowRect.left;
}


UINT Window::GetY() const
{
    return data1_.windowRect.top;
}


UINT Window::GetWidth() const
{
    return data1_.windowRect.right - data1_.windowRect.left;
}


UINT Window::GetHeight() const
{
    return data1_.windowRect.bottom - data1_.windowRect.top;
}


UINT Window::GetClientWidth() const
{
    return data1_.clientRect.right - data1_.clientRect.left;
}


UINT Window::GetClientHeight() const
{
    return data1_.clientRect.bottom - data1_.clientRect.top;
}


UINT Window::GetZOrder() const
{
    return data1_.zOrder;
}


BYTE* Window::GetBuffer() const
{
    return windowTexture_->GetBuffer();
}


UINT Window::GetTextureWidth() const
{
    return windowTexture_->GetWidth();
}


UINT Window::GetTextureHeight() const
{
    return windowTexture_->GetHeight();
}


UINT Window::GetTextureOffsetX() const
{
    return windowTexture_->GetOffsetX();
}


UINT Window::GetTextureOffsetY() const
{
    return windowTexture_->GetOffsetY();
}


UINT Window::GetIconWidth() const
{
    return iconTexture_->GetWidth();
}


UINT Window::GetIconHeight() const
{
    return iconTexture_->GetHeight();
}


const std::wstring& Window::GetTitle() const
{
    return data2_.title;
}


const std::string& Window::GetClass() const
{
    return data2_.className;
}


void Window::SetWindowTexture(ID3D11Texture2D* ptr)
{
    windowTexture_->SetUnityTexturePtr(ptr);
}


ID3D11Texture2D* Window::GetWindowTexture() const
{
    return windowTexture_->GetUnityTexturePtr();
}


void Window::SetIconTexture(ID3D11Texture2D* ptr)
{
    iconTexture_->SetUnityTexturePtr(ptr);
}


ID3D11Texture2D* Window::GetIconTexture() const
{
    return iconTexture_->GetUnityTexturePtr();
}


void Window::SetCaptureMode(CaptureMode mode)
{
    windowTexture_->SetCaptureMode(mode);
}


void Window::SetCursorDraw(bool draw)
{
    windowTexture_->SetCursorDraw(draw);
}


bool Window::GetCursorDraw() const
{
    return windowTexture_->GetCursorDraw();
}


UINT Window::GetPixel(int x, int y) const
{
    return windowTexture_->GetPixel(x, y);
}


bool Window::GetPixels(BYTE* output, int x, int y, int width, int height) const
{
    return windowTexture_->GetPixels(output, x, y, width, height);
}


CaptureMode Window::GetCaptureMode() const
{
    return windowTexture_->GetCaptureMode();
}


bool Window::IsJustAdded() const
{
    return frameCount_ == 0;
}


void Window::UpdateFrameCount()
{
    ++frameCount_;
}


void Window::RequestUpdateTitle()
{
    hasTitleUpdateRequested_ = true;
}


void Window::UpdateTitle()
{
    if (!IsDesktop())
    {
        if (windowTexture_->IsWindowsGraphicsCaptureAvailable())
        {
            if (const auto wgc = windowTexture_->GetWindowsGraphicsCapture())
            {
                data2_.title = wgc->GetDisplayName();
            }
        }
        else
        {
            constexpr UINT timeout = 100 /* milliseconds */;
            GetWindowTitle(data1_.hWnd, data2_.title, timeout);
        }
    }
    else
    {
        MONITORINFOEX monitor;
        monitor.cbSize = sizeof(MONITORINFOEX);
        if (::GetMonitorInfo(data1_.hMonitor, &monitor))
        {
            WCHAR buf[_countof(monitor.szDevice)];
            size_t len;
            mbstowcs_s(&len, buf, _countof(monitor.szDevice), monitor.szDevice, _TRUNCATE);
            data2_.title = buf;
        }
    }
}


void Window::UpdateIsBackground()
{
    if (IsApplicationFrameWindow())
    {
        data2_.isBackground = IsCloakedWindow(GetWindowHandle());
    }
    else
    {
        data2_.isBackground = false;
    }
}


void Window::Capture()
{
    // Run this scope in the thread loop managed by CaptureManager.

    if (hasNewWindowTextureCaptured_)
    {
        // If it is called before Upload(), skip this frame.
        return;
    }

    if (!IsWindow() || !IsVisible())
    {
        return;
    }

    UWC_SCOPE_TIMER(WindowCapture)

    if (windowTexture_->Capture())
    {
        hasNewWindowTextureCaptured_ = true;

        if (auto& uploader = WindowManager::GetUploadManager())
        {
            uploader->RequestUploadWindow(id_);
        }
    }
}


void Window::Upload()
{
    // Run this scope in the thread loop managed by UploadManager.
    if (windowTexture_->Upload())
    {
        hasNewWindowTextureUploaded_ = true;
    }

    hasNewWindowTextureCaptured_ = false;
}


void Window::CaptureIcon()
{
    if (!IsWindow())
    {
        return;
    }

    if (!iconTexture_->CaptureOnce())
    {
        return;
    }

    if (auto& uploader = WindowManager::GetUploadManager())
    {
        uploader->RequestUploadIcon(id_);
    }
}


void Window::UploadIcon()
{
    if (iconTexture_->UploadOnce())
    {
        hasNewIconTextureUploaded_ = true;
    }
}


void Window::RenderIcon()
{
    iconTexture_->RenderOnce();
}


void Window::Render()
{
    // Run this scope in the unity rendering thread.

    if (hasNewWindowTextureUploaded_)
    {
        hasNewWindowTextureUploaded_ = false;
        windowTexture_->Render();
        hasNewWindowTextureCaptured_ = false;
    }

    if (hasNewIconTextureUploaded_)
    {
        hasNewIconTextureUploaded_ = false;
        iconTexture_->RenderOnce();
    }
}
