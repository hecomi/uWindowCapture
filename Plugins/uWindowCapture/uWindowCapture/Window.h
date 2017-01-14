#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <mutex>
#include <atomic>

#include "Buffer.h"
#include "Thread.h"


class Window
{
friend class WindowManager;
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

    HWND GetHandle() const;
    HWND GetOwner() const;

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetZOrder() const;

    void UpdateTitle();
    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetTexturePtr() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void StartCapture();
    void StopCapture();
    void RequestCapture();
    void Render();
    void UploadTextureToGpu();

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
    void RequestUpload();

    CaptureMode mode_ = CaptureMode::PrintWindow;

    const int id_ = -1;
    const HWND window_ = nullptr;
    HWND owner_ = nullptr;
    std::wstring title_;

    RECT rect_;
    UINT zOrder_ = 0;

    ThreadLoop captureThreadLoop_;
    std::mutex mutex_;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_;
    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;

    Buffer<BYTE> buffer_;
    HBITMAP bitmap_ = nullptr;
    std::atomic<UINT> bufferWidth_ = 0;
    std::atomic<UINT> bufferHeight_ = 0;

    std::atomic<bool> isCaptureRequested_ = false;
    std::atomic<bool> hasNewTextureUploaded_ = false;

    std::atomic<bool> isAlive_ = true;
    std::atomic<bool> isDesktop_ = false;
    std::atomic<bool> isAltTabWindow_ = false;
};

