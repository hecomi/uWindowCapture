#pragma once

#include <queue>
#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "Device.h"
#include "Debug.h"
#include "Common.h"

#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;



IsolatedD3D11Device::IsolatedD3D11Device()
{
}


IsolatedD3D11Device::~IsolatedD3D11Device()
{
}


HRESULT IsolatedD3D11Device::Create(const ComPtr<IDXGIAdapter>& adapter)
{
    const auto driverType = adapter ? 
        D3D_DRIVER_TYPE_UNKNOWN : 
        D3D_DRIVER_TYPE_HARDWARE;
    const auto flags =
        D3D11_CREATE_DEVICE_BGRA_SUPPORT;       // D2D Compatible
        // | D3D11_CREATE_DEVICE_VIDEO_SUPPORT  // MediaFoundation
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

    return D3D11CreateDevice(
        adapter.Get(),
        driverType,
        nullptr,
        flags,
        featureLevelsRequested,
        numLevelsRequested,
        D3D11_SDK_VERSION,
        &device_,
        &featureLevelsSupported,
        nullptr);
}


ComPtr<ID3D11Device> IsolatedD3D11Device::GetDevice()
{ 
    return device_;
}


Microsoft::WRL::ComPtr<ID3D11Texture2D> IsolatedD3D11Device::CreateSharedTexture(UINT width, UINT height)
{
    ComPtr<ID3D11Texture2D> texture;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width              = width;
    desc.Height             = height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DYNAMIC;
    desc.BindFlags          = 0;
    desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags          = D3D11_RESOURCE_MISC_SHARED;

    if (FAILED(device_->CreateTexture2D(&desc, nullptr, &texture)))
    {
        Debug::Error(__FUNCTION__, " => GetDevice()->CreateTexture2D() failed.");
        return nullptr;
    }

    return texture;
}