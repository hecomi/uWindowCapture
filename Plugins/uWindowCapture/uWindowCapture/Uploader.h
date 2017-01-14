#pragma once

#include <set>
#include <mutex>
#include <d3d11.h>
#include <wrl/client.h>

#include "Thread.h"
#include "Singleton.h"


class Window;


class Uploader
{
    UWC_SINGLETON(Uploader)

public:
    using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;
    using TexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;

    void Initialize();
    void Finalize();

    DevicePtr GetDevice();
    TexturePtr CreateCompatibleSharedTexture(const TexturePtr& texture);
    void RequestUploadInBackgroundThread(int id);
    void StartUploadThread();
    void StopUploadThread();
    void UploadTextures();

private:
    DevicePtr device_;
    ThreadLoop threadLoop_;
    std::set<int> uploadList_;
    mutable std::mutex mutex_;
};
