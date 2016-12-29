#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <thread>
#include <mutex>
#include "Common.h"

class Window
{
public:
    enum class CaptureMode
    {
        PrintWindow = 0,
        BitBlt = 1,
    };

    explicit Window(HWND hwnd);
    ~Window();

    BOOL IsWindow() const;
    BOOL IsVisible() const;
    HWND GetHandle() const;
    RECT GetRect() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    void GetTitle(WCHAR* buf, int len) const;
    void SetTexturePtr(ID3D11Texture2D* ptr);
    void SetCaptureMode(CaptureMode mode);

    void Capture();
    void Draw();

private:
    void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
    void DeleteBitmap();
    void CaptureInternal();

    CaptureMode mode_ = CaptureMode::PrintWindow;
    std::thread captureThread_;
    std::mutex mutex_;
    bool hasCaptureFinished_ = true;
    HWND window_ = nullptr;
    Buffer<BYTE> buffer_;
    HBITMAP bitmap_ = nullptr;
    UINT width_ = 0;
    UINT height_ = 0;
    ID3D11Texture2D* texture_ = nullptr;
};

