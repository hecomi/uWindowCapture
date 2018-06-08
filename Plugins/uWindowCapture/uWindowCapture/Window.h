#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <atomic>

#include "Buffer.h"


enum class CaptureMode;


class Window
{
friend class WindowManager;
public:
    struct Data
    {
        HWND hWnd;
        RECT windowRect;
        RECT clientRect;
        UINT zOrder;
        HWND hOwner;
        HWND hParent;
        HINSTANCE hInstance;
        DWORD processId;
        DWORD threadId;
        std::wstring title;
        HMONITOR hMonitor;
        BOOL isAltTabWindow;
        BOOL isDesktop;
    };

    explicit Window(int id);
    ~Window();

    void SetData(Data&& data);

    int GetId() const;
    int GetParentId() const;
    HWND GetHandle() const;
    HWND GetOwnerHandle() const;
    HWND GetParentHandle() const;
    HINSTANCE GetInstance() const;
    DWORD GetProcessId() const;
    DWORD GetThreadId() const;

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetClientWidth() const;
    UINT GetClientHeight() const;
    UINT GetZOrder() const;
    UINT GetBufferWidth() const;
    UINT GetBufferHeight() const;
    UINT GetIconWidth() const;
    UINT GetIconHeight() const;

    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetWindowTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetWindowTexture() const;

    void SetIconTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetIconTexture() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void Capture();
    void Upload();
    void Render();

    void CaptureIcon();
    void UploadIcon();
    void RenderIcon();

    bool IsAltTab() const;
    bool IsDesktop() const;
    BOOL IsWindow() const;
    BOOL IsVisible() const;
    BOOL IsEnabled() const;
    BOOL IsUnicode() const;
    BOOL IsZoomed() const;
    BOOL IsIconic() const;
    BOOL IsHungUp() const;
    BOOL IsTouchable() const;

    BOOL MoveWindow(int x, int y);
    BOOL ScaleWindow(int width, int height);
    BOOL MoveAndScaleWindow(int x, int y, int width, int height);

private:
    std::shared_ptr<class WindowTexture> windowTexture_ = std::make_shared<WindowTexture>(this);
    std::shared_ptr<class IconTexture> iconTexture_ = std::make_shared<IconTexture>(this);
    Data data_;

    const int id_ = -1;
    int parentId_ = -1;
    int frameCount_ = 0;

    std::atomic<bool> hasNewWindowTextureUploaded_ = false;
    std::atomic<bool> hasNewIconTextureUploaded_ = false;
    std::atomic<bool> isAlive_ = true;
    std::atomic<bool> isAltTabWindow_ = false;
};
