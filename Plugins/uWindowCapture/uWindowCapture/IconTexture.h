#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <mutex>
#include <atomic>

#include "Buffer.h"


class Window;


class IconTexture
{
public:
    explicit IconTexture(Window* window);
    ~IconTexture();

    bool IsValid() const { return hIcon_; }
    UINT GetWidth() const;
    UINT GetHeight() const;

    void SetUnityTexturePtr(ID3D11Texture2D* ptr);
    ID3D11Texture2D* GetUnityTexturePtr() const;

    bool Capture();
    bool CaptureOnce();
    bool Upload();
    bool UploadOnce();
    bool Render();
    bool RenderOnce();

private:
    void InitIcon();
    void InitIconHandle();

    Window* const window_ = nullptr;
    HICON hIcon_ = nullptr;

    std::atomic<ID3D11Texture2D*> unityTexture_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> sharedTexture_;
    HANDLE sharedHandle_ = nullptr;
    std::mutex sharedTextureMutex_;

    Buffer<BYTE> buffer_;
    std::mutex bufferMutex_;

    std::atomic<bool> hasCaptured_ = false;
    std::atomic<bool> hasUploaded_ = false;
    std::atomic<bool> hasRendered_ = false;

    UINT width_ = 0;
    UINT height_ = 0;
};