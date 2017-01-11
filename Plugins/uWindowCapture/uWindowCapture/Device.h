#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>


// Thraed safe self created ID3D11Device from specified adapter
class IsolatedD3D11Device
{
public:
    IsolatedD3D11Device();
    ~IsolatedD3D11Device();

    Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
    Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateCompatibleSharedTexture(
        const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);

private:
    void Create();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    UINT width_ = 0;
    UINT height_ = 0;
};
