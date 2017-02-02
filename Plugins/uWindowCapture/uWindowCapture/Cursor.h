#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <mutex>
#include <atomic>

#include "Buffer.h"
#include "Thread.h"


class Cursor
{
public:
    Cursor();
    ~Cursor();

    void StartCapture();
    void StopCapture();

    UINT GetX() const;
    UINT GetY() const;
    UINT GetWidth() const;
    UINT GetHeight() const;

    void SetUnityTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetUnityTexturePtr() const;

    bool Capture();
    bool HasCaptured() const;
    bool Upload();
    bool HasUploaded() const;
    bool Render();

private:
    void CreateBitmapIfNeeded(HDC hDc, UINT width, UINT height);
    void DeleteBitmap();

    ThreadLoop threadLoop_;

    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_;
    std::mutex sharedTextureMutex_;

    Buffer<BYTE> buffer_;
    HBITMAP bitmap_ = nullptr;
    std::mutex bufferMutex_;

    std::atomic<UINT> width_ = 0;
    std::atomic<UINT> height_ = 0;
    std::atomic<UINT> x_ = 0;
    std::atomic<UINT> y_ = 0;
    std::mutex cursorMutex_;

    std::atomic<bool> hasCaptured_ = false;
    std::atomic<bool> hasUploaded_ = false;
};