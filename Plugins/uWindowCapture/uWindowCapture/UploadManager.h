#pragma once

#include <atomic>
#include <d3d11.h>
#include <wrl/client.h>

#include "WindowQueue.h"
#include "Thread.h"


class Window;


class UploadManager
{
public:
    using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;
    using TexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;

    UploadManager();
    ~UploadManager();

    DevicePtr GetDevice();
    TexturePtr CreateCompatibleSharedTexture(const TexturePtr& texture);
    void RequestUpload(int id);
    void StartUploadThread();
    void StopUploadThread();
    void TriggerGpuUpload();

private:
    DevicePtr device_;
    std::thread initThread_;
    ThreadLoop threadLoop_;
    WindowQueue uploadQueue_;
    std::atomic<bool> hasUploadTriggered_ = false;
};
