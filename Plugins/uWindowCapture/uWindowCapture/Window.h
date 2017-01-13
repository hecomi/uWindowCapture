#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

#include "Common.h"


class Window
{
friend WindowManager;
public:
    enum class CaptureMode
    {
        None = -1,
        PrintWindow = 0,
        BitBlt = 1,
        BitBltAlpha = 2,
    };

    Window(HWND hwnd, int id);
    ~Window();
    void Update();

    HWND GetHandle() const;
    HWND GetOwner() const;
    RECT GetRect() const;
    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetZOrder() const;
    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetTexturePtr() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void Capture();
    void Render();
    void UploadTextureToGpu(const std::shared_ptr<class IsolatedD3D11Device>& device);

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

private:
    void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
    void DeleteBitmap();
    void CaptureInternal();

    CaptureMode mode_ = CaptureMode::PrintWindow;

    std::thread captureThread_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_;
    std::atomic<bool> hasCaptureFinished_ = true;
    std::atomic<bool> hasNewTextureUploaded_ = false;
    std::atomic<bool> hasCaptureMessageSent_ = true;

    int id_ = -1;
    HWND window_ = nullptr;
    HWND owner_ = nullptr;
    Buffer<BYTE> buffer_;
    HBITMAP bitmap_ = nullptr;
    std::atomic<UINT> width_ = 0;
    std::atomic<UINT> height_ = 0;
    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;
    std::wstring title_;
    std::mutex mutex_;

    bool isAlive_ = true;
    bool isDesktop_ = false;
    bool isAltTabWindow_ = false;
};

