#include "Window.h"
#include "WindowTexture.h"
#include "IconTexture.h"
#include "WindowManager.h"
#include "Debug.h"
#include "Util.h"



Window::Window(int id)
    : id_(id)
{
}


Window::~Window()
{
}


void Window::SetData(Data&& data)
{
    data_ = data;
}


int Window::GetId() const
{
    return id_;
}


int Window::GetParentId() const
{
    return parentId_;
}


HWND Window::GetHandle() const
{
    return data_.hWnd;
}


HWND Window::GetOwnerHandle() const
{
    return data_.hOwner;
}


HWND Window::GetParentHandle() const
{
    return data_.hParent;
}


HINSTANCE Window::GetInstance() const
{
    return data_.hInstance;
}


DWORD Window::GetProcessId() const
{
    return data_.processId;
}


DWORD Window::GetThreadId() const
{
    return data_.threadId;
}


const RECT & Window::GetWindowRect() const
{
    return data_.windowRect;
}


const RECT & Window::GetClientRect() const
{
    return data_.clientRect;
}


bool Window::IsAltTab() const
{
    return data_.isAltTabWindow;
}


bool Window::IsDesktop() const
{
    return data_.isDesktop;
}


BOOL Window::IsWindow() const
{
    return ::IsWindow(GetHandle());
}


BOOL Window::IsVisible() const
{
    return ::IsWindowVisible(GetHandle());
}


BOOL Window::IsEnabled() const
{
    return ::IsWindowEnabled(GetHandle());
}


BOOL Window::IsUnicode() const
{
    return ::IsWindowUnicode(GetHandle());
}


BOOL Window::IsZoomed() const
{
    return ::IsZoomed(GetHandle());
}


BOOL Window::IsIconic() const
{
    return ::IsIconic(GetHandle());
}


BOOL Window::IsHungUp() const
{
    return ::IsHungAppWindow(GetHandle());
}


BOOL Window::IsTouchable() const
{
    return ::IsTouchWindow(GetHandle(), NULL);
}


BOOL Window::IsStoreApp() const
{
    return data_.isStoreApp;
}


BOOL Window::IsBackground() const
{
    return data_.isBackground;
}


BOOL Window::MoveWindow(int x, int y)
{
    return MoveAndScaleWindow(x, y, GetWidth(), GetHeight());
}


BOOL Window::ScaleWindow(int width, int height)
{
    return MoveAndScaleWindow(GetX(), GetY(), width, height);
}


BOOL Window::MoveAndScaleWindow(int x, int y, int width, int height)
{
    bool repaint = (width != GetWidth() || height != GetHeight());
    return ::MoveWindow(GetHandle(), x, y, width, height, repaint);
}


UINT Window::GetX() const
{
    return data_.windowRect.left;
}


UINT Window::GetY() const
{
    return data_.windowRect.top;
}


UINT Window::GetWidth() const
{
    return data_.windowRect.right - data_.windowRect.left;
}


UINT Window::GetHeight() const
{
    return data_.windowRect.bottom - data_.windowRect.top;
}


UINT Window::GetClientWidth() const
{
    return data_.clientRect.right - data_.clientRect.left;
}


UINT Window::GetClientHeight() const
{
    return data_.clientRect.bottom - data_.clientRect.top;
}


UINT Window::GetZOrder() const
{
    return data_.zOrder;
}


UINT Window::GetBufferWidth() const
{
    return windowTexture_->GetWidth();
}


UINT Window::GetBufferHeight() const
{
    return windowTexture_->GetHeight();
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
    return data_.title;
}


const std::string& Window::GetClass() const
{
    return data_.className;
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


CaptureMode Window::GetCaptureMode() const
{
    return windowTexture_->GetCaptureMode();
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
        if (auto& uploader = WindowManager::GetUploadManager())
        {
            hasNewIconTextureUploaded_ = true;
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
