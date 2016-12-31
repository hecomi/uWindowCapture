#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <thread>
#include <mutex>
#include <list>
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
    void Update();

    BOOL IsWindow() const;
    BOOL IsVisible() const;
    HWND GetHandle() const;
    std::shared_ptr<Window> GetParent();
    void SetParent(const std::shared_ptr<Window>& parent);
    RECT GetRect() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetTitleLength() const;
    const std::wstring& GetTitle() const;
    void SetTitle(const WCHAR* title);
    void SetTexturePtr(ID3D11Texture2D* ptr);
    void SetCaptureMode(CaptureMode mode);

    void Capture();
    void Draw();
    
    // void UpdateChildWindows();
    // void AddChild(HWND hWnd);
    void SetAlive(bool isAlive);
    bool IsAlive() const;

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
    std::wstring name_;

    std::weak_ptr<Window> parent_;
    bool isAlive_ = true;
};

