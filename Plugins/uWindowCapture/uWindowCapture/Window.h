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
    struct Data1
    {
        BOOL isDesktop;
        HWND hWnd;
        HMONITOR hMonitor;
        HWND hOwner;
        RECT windowRect;
        RECT clientRect;
        UINT zOrder;
    };

    struct Data2
    {
        HWND hParent;
        HINSTANCE hInstance;
        DWORD processId;
        DWORD threadId;
        std::wstring title;
        std::string className;
        BOOL isAltTabWindow;
        BOOL isStoreApp;
        BOOL isBackground;
    };

    explicit Window(int id);
    ~Window();

    void SetData(const Data1& data);

    int GetId() const;
    int GetParentId() const;
    HWND GetHandle() const;
    HWND GetOwnerHandle() const;
    HWND GetParentHandle() const;
    HINSTANCE GetInstance() const;
    DWORD GetProcessId() const;
    DWORD GetThreadId() const;

    const RECT & GetWindowRect() const;
    const RECT & GetClientRect() const;

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

    const std::wstring& GetTitle() const;
    const std::string& GetClass() const;

    void SetWindowTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetWindowTexture() const;

    void SetIconTexture(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetIconTexture() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void SetCursorDraw(bool draw);
    bool GetCursorDraw() const;

    void RequestUpdateTitle();

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
    BOOL IsStoreApp() const;
    BOOL IsBackground() const;

private:
    void UpdateTitle();
    void UpdateIsBackground();

    std::shared_ptr<class WindowTexture> windowTexture_ = std::make_shared<WindowTexture>(this);
    std::shared_ptr<class IconTexture> iconTexture_ = std::make_shared<IconTexture>(this);
    Data1 data1_;
    Data2 data2_;

    const int id_ = -1;
    int parentId_ = -1;
    int frameCount_ = 0;

    std::atomic<bool> hasTitleUpdateRequested_ = false;
    std::atomic<bool> hasNewWindowTextureCaptured_ = false;
    std::atomic<bool> hasNewWindowTextureUploaded_ = false;
    std::atomic<bool> hasNewIconTextureUploaded_ = false;
    std::atomic<bool> isAlive_ = true;
};
