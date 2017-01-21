#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <mutex>
#include <atomic>

#include "Buffer.h"


enum class CaptureMode
{
    None = -1,
    PrintWindow = 0,
    BitBlt = 1,
    BitBltAlpha = 2,
};


class Window
{
friend class WindowManager;
public:
    Window(HWND hwnd, int id);
    ~Window();

    HWND GetHandle() const;
    HWND GetOwner() const;
    HWND GetParent() const;
    HINSTANCE GetInstance() const;
    DWORD GetProcessId() const;
    DWORD GetThreadId() const;

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetZOrder() const;
    UINT GetBufferWidth() const;
    UINT GetBufferHeight() const;

    void UpdateTitle();
    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;

    void SetTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetTexturePtr() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void Capture();
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

    BOOL MoveWindow(int x, int y);
    BOOL ScaleWindow(int width, int height);
    BOOL MoveAndScaleWindow(int x, int y, int width, int height);

private:
    void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
    void DeleteBitmap();
    BOOL CaptureInternal();
    void RequestUpload();

    CaptureMode mode_ = CaptureMode::PrintWindow;

    const int id_ = -1;
    const HWND window_ = nullptr;
    HWND owner_ = nullptr;
    HWND parent_ = nullptr;
    HINSTANCE instance_ = nullptr;
    DWORD processId_ = -1;
    DWORD threadId_ = -1;
    std::wstring title_ = L"";

    RECT rect_;
    UINT zOrder_ = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_;
    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;
    std::mutex sharedTextureMutex_;

    Buffer<BYTE> buffer_;
    HBITMAP bitmap_ = nullptr;
    std::atomic<UINT> bufferWidth_ = 0;
    std::atomic<UINT> bufferHeight_ = 0;
    std::mutex bufferMutex_;

    std::atomic<bool> hasNewTextureUploaded_ = false;
    std::atomic<bool> isAlive_ = true;
    std::atomic<bool> isDesktop_ = false;
    std::atomic<bool> isAltTabWindow_ = false;
};
