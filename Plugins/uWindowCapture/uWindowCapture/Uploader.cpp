#pragma once

#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Uploader.h"
#include "WindowManager.h"
#include "Debug.h"
#include "Window.h"
#include "Unity.h"

#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;
using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;
using TexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;



UWC_SINGLETON_INSTANCE(Uploader)


void Uploader::Initialize()
{
    ComPtr<IDXGIDevice1> dxgiDevice;
    if (FAILED(GetUnityDevice()->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))){
        Debug::Error(__FUNCTION__, " => QueryInterface from IUnityGraphicsD3D11 to IDXGIDevice1 failed.");
        return;
    }

    ComPtr<IDXGIAdapter> dxgiAdapter;
    if (FAILED(dxgiDevice->GetAdapter(&dxgiAdapter))) {
        Debug::Error(__FUNCTION__, " => QueryInterface from IDXGIDevice1 to IDXGIAdapter failed.");
        return;
    }

    const auto driverType = D3D_DRIVER_TYPE_UNKNOWN;
    const auto flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    const D3D_FEATURE_LEVEL featureLevelsRequested[] = 
    {
        D3D_FEATURE_LEVEL_11_0, 
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0, 
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2, 
        D3D_FEATURE_LEVEL_9_1 
    };
    const UINT numLevelsRequested = sizeof(featureLevelsRequested) / sizeof(D3D_FEATURE_LEVEL);
    D3D_FEATURE_LEVEL featureLevelsSupported;

    D3D11CreateDevice(
        dxgiAdapter.Get(),
        driverType,
        nullptr,
        flags,
        featureLevelsRequested,
        numLevelsRequested,
        D3D11_SDK_VERSION,
        &device_,
        &featureLevelsSupported,
        nullptr);

    StartUploadThread();
}


void Uploader::Finalize()
{
    StopUploadThread();
}


DevicePtr Uploader::GetDevice()
{ 
    return device_;
}


TexturePtr Uploader::CreateCompatibleSharedTexture(const TexturePtr& texture)
{
    TexturePtr sharedTexture;

    D3D11_TEXTURE2D_DESC srcDesc;
    texture->GetDesc(&srcDesc);

    D3D11_TEXTURE2D_DESC desc = srcDesc;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    if (FAILED(device_->CreateTexture2D(&desc, nullptr, &sharedTexture)))
    {
        Debug::Error(__FUNCTION__, " => GetDevice()->CreateTexture2D() failed.");
        return nullptr;
    }

    return sharedTexture;
}


void Uploader::StartUploadThread()
{
    threadLoop_.Start([this] 
    { 
        UploadTextures(); 
    });
}


void Uploader::StopUploadThread()
{
    threadLoop_.Stop();
}


void Uploader::RequestUploadInBackgroundThread(int id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uploadList_.insert(id);
}


void Uploader::UploadTextures()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto id : uploadList_)
    {
        if (auto window = WindowManager::Get().GetWindow(id))
        {
            window->UploadTextureToGpu();
        }
    }

    uploadList_.clear();
}


