#pragma once

#include <queue>
#include <d3d9.h>
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
    Create();
}


IsolatedD3D11Device::~IsolatedD3D11Device()
{
}


void IsolatedD3D11Device::Create()
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
}


ComPtr<ID3D11Device> IsolatedD3D11Device::GetDevice()
{ 
    return device_;
}


ComPtr<ID3D11Texture2D> IsolatedD3D11Device::CreateCompatibleSharedTexture(const ComPtr<ID3D11Texture2D>& texture)
{
    ComPtr<ID3D11Texture2D> sharedTexture;

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
