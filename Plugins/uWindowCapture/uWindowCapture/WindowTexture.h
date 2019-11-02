#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <mutex>
#include <atomic>

#include "Buffer.h"


enum class CaptureMode
{
    None = -1,
    PrintWindow = 0,
    BitBlt = 1,
};


class Window;


class WindowTexture
{
public:
    explicit WindowTexture(Window* window);
    ~WindowTexture();

    void SetUnityTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetUnityTexturePtr() const;

    void SetCaptureMode(CaptureMode mode);
    CaptureMode GetCaptureMode() const;

    void SetCursorDraw(bool draw);
    bool GetCursorDraw() const;

    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetOffsetX() const;
    UINT GetOffsetY() const;

    bool Capture();
    bool Upload();
    bool Render();

    BYTE* GetBuffer();

    UINT GetPixel(int x, int y) const;
    bool GetPixels(BYTE* output, int x, int y, int width, int height) const;

private:
    void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
    void DeleteBitmap();
    void DrawCursor(HWND hWnd, HDC hDcMem);

    const Window* const window_;
    CaptureMode captureMode_ = CaptureMode::PrintWindow;

    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_;
    std::mutex sharedTextureMutex_;

    Buffer<BYTE> buffer_;
    Buffer<BYTE> bufferForGetBuffer_;
    HBITMAP bitmap_ = nullptr;
    std::atomic<UINT> bufferWidth_ = 0;
    std::atomic<UINT> bufferHeight_ = 0;
    std::atomic<UINT> offsetX_ = 0;
    std::atomic<UINT> offsetY_ = 0;
    std::atomic<UINT> textureWidth_ = 0;
    std::atomic<UINT> textureHeight_ = 0;
    std::atomic<bool> drawCursor_ = true;
    mutable std::mutex bufferMutex_;

    float dpiScaleX_ = 1.f;
    float dpiScaleY_ = 1.f;
};