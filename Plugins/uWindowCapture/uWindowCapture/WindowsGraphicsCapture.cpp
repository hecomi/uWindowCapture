#include "WindowsGraphicsCapture.h"
#include "WindowManager.h"
#include "UploadManager.h"
#include "Debug.h"
#include <inspectable.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <windows.foundation.h>
#include <Windows.h>

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace ::Windows::Graphics::DirectX::Direct3D11;

#pragma comment(lib, "windowsapp")


bool WindowsGraphicsCapture::IsAvailable()
{
    using ApiInfo = winrt::Windows::Foundation::Metadata::ApiInformation;

    static bool isChecked = false;
    static bool isAvailable = false;
    if (isChecked) return isAvailable;
    isChecked = true;

    try
    {
        isAvailable = ApiInfo::IsApiContractPresent(L"Windows.Foundation.UniversalApiContract", 8);
    }
    catch (...)
    {
        Debug::Log("WindowsGraphicsCapture::IsAvailable() throws an exception.");
    }

    return isAvailable;
}


bool WindowsGraphicsCapture::IsCursorCaptureEnabledApiAvailable()
{
    using ApiInfo = winrt::Windows::Foundation::Metadata::ApiInformation;

    static bool isChecked = false;
    static bool isEnabled = false;
    if (isChecked) return isEnabled;
    isChecked = true;

    try
    {
        isEnabled = ApiInfo::IsPropertyPresent(
            L"Windows.Graphics.Capture.GraphicsCaptureSession",
            L"IsCursorCaptureEnabled");
    }
    catch (...)
    {
        Debug::Log("WindowsGraphicsCapture::IsCursorCaptureEnabledApiAvailable() throws an exception.");
    }

    return isEnabled;
}


WindowsGraphicsCapture::WindowsGraphicsCapture(HWND hWnd)
    : hWnd_(hWnd)
    , hMonitor_(NULL)
{
}


WindowsGraphicsCapture::WindowsGraphicsCapture(HMONITOR hMonitor)
    : hMonitor_(hMonitor)
    , hWnd_(NULL)
{
}


void WindowsGraphicsCapture::Initialize()
{
    auto device = WindowManager::Get().GetUploadManager()->GetDevice();

    com_ptr<IDXGIDevice> dxgiDevice;
    const auto hr = device->QueryInterface<IDXGIDevice>(dxgiDevice.put());
    if (FAILED(hr)) return;

    com_ptr<::IInspectable> deviceWinRt;
    ::CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), deviceWinRt.put());
    deviceWinRt_ = deviceWinRt.as<IDirect3DDevice>();

    const auto factory = get_activation_factory<GraphicsCaptureItem>();
    const auto interop = factory.as<IGraphicsCaptureItemInterop>();
    if (hWnd_)
    {
        interop->CreateForWindow(
            hWnd_,
            guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), 
            reinterpret_cast<void**>(put_abi(graphicsCaptureItem_)));
    }
    else
    {
        interop->CreateForMonitor(
            hMonitor_,
            guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), 
            reinterpret_cast<void**>(put_abi(graphicsCaptureItem_)));
    }

    size_ = graphicsCaptureItem_.Size();
    captureFramePool_ = Direct3D11CaptureFramePool::CreateFreeThreaded(
        deviceWinRt_, 
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        size_);

    graphicsCaptureSession_ = captureFramePool_.CreateCaptureSession(graphicsCaptureItem_);
}


WindowsGraphicsCapture::~WindowsGraphicsCapture()
{
    assert(!isStarted_);
    graphicsCaptureItem_ = nullptr;
    deviceWinRt_ = nullptr;
}


void WindowsGraphicsCapture::Start()
{
    if (isStarted_) return;
    isStarted_ = true;

    Initialize();

    if (graphicsCaptureSession_)
    {
        graphicsCaptureSession_.StartCapture();
    }

    if (const auto& manager = WindowManager::Get().GetWindowsGraphicsCaptureManager())
    {
        manager->Add(shared_from_this());
    }
}


void WindowsGraphicsCapture::RequestStop()
{
    hasStopRequested_ = true;
}


void WindowsGraphicsCapture::Stop()
{
    if (!isStarted_) return;
    isStarted_ = false;

    if (graphicsCaptureSession_)
    {
        graphicsCaptureSession_.Close();
        graphicsCaptureSession_ = nullptr;
    }

    if (captureFramePool_)
    {
        captureFramePool_.Close();
        captureFramePool_ = nullptr;
    }

    if (const auto& manager = WindowManager::Get().GetWindowsGraphicsCaptureManager())
    {
        manager->Remove(shared_from_this());
    }
}


void WindowsGraphicsCapture::EnableCursorCapture(bool enabled)
{
    if (!IsCursorCaptureEnabledApiAvailable())
    {
        Debug::Log("CursorCaptureEnabled API is not available.");
        return;
    }

    if (graphicsCaptureSession_) {
        graphicsCaptureSession_.IsCursorCaptureEnabled(enabled);
    }
}


WindowsGraphicsCapture::Result WindowsGraphicsCapture::TryGetLatestResult()
{
    stopTimer_ = 0.f;

    if (!captureFramePool_) return {};

    Direct3D11CaptureFrame frame = nullptr;
    while (const auto nextFrame = captureFramePool_.TryGetNextFrame())
    {
        frame = nextFrame;
    }
    if (!frame) return {};

    const auto surface = frame.Surface();
    if (!surface) return {};

    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    com_ptr<ID3D11Texture2D> texture;
    const auto hr = access->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void());
    if (FAILED(hr)) return {};

    const auto size = frame.ContentSize();
    const bool hasSizeChanged = 
        (size_.Width != size.Width) || 
        (size_.Height != size.Height);

    return { texture.get(), size.Width, size.Height, hasSizeChanged };
}


void WindowsGraphicsCapture::ChangePoolSize(int width, int height)
{
    size_ = { width, height };
    captureFramePool_.Recreate(
        deviceWinRt_, 
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        size_);
}


// ---


void WindowsGraphicsCaptureManager::Add(const std::shared_ptr<WindowsGraphicsCapture>& instance)
{
    std::scoped_lock lock(instancesMutex_);
    instances_.push_back(instance);
}


void WindowsGraphicsCaptureManager::Remove(const std::shared_ptr<WindowsGraphicsCapture>& instance)
{
    std::scoped_lock lock(instancesMutex_);
    instances_.erase(
        std::remove(
            instances_.begin(),
            instances_.end(),
            instance),
        instances_.end());
}


void WindowsGraphicsCaptureManager::Update(float dt)
{
    std::scoped_lock lock(instancesMutex_);

    std::vector<Ptr> toBeStoppedInstances;
    for (const auto& instance : instances_)
    {
        if (instance->stopTimer_ > 1.f || instance->hasStopRequested_) {
            std::scoped_lock lock(removedInstancesMutex_);
            removedInstances_.push_back(instance);
            toBeStoppedInstances.push_back(instance);
            continue;
        }
        instance->stopTimer_ += dt;
    }
}


void WindowsGraphicsCaptureManager::StopNonUpdatedInstances()
{
    std::scoped_lock lock(removedInstancesMutex_);

    for (auto &instance : removedInstances_)
    {
        instance->Stop();
    }

    removedInstances_.clear();
}


void WindowsGraphicsCaptureManager::StopAllInstances()
{
    StopNonUpdatedInstances();

    decltype(instances_) instances;
    {
        std::scoped_lock lock(instancesMutex_);
        instances = instances_;
        instances_.clear();
    }

    for (auto &instance : instances)
    {
        instance->Stop();
    }
}
