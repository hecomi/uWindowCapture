#include "WindowsGraphicsCapture.h"
#include "WindowManager.h"
#include "UploadManager.h"
#include "Debug.h"
#include "Util.h"
#include "Window.h"
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

#pragma comment(lib, "windowsapp")

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace ::Windows::Graphics::DirectX::Direct3D11;


bool WindowsGraphicsCapture::IsSupported()
{
    return GraphicsCaptureSession::IsSupported();
}


bool WindowsGraphicsCapture::IsCursorCaptureEnabledApiSupported()
{
    using ApiInfo = winrt::Windows::Foundation::Metadata::ApiInformation;

    if (!IsSupported()) return false;

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
    catch (const std::exception &e)
    {
        Debug::Log("WindowsGraphicsCapture::IsCursorCaptureEnabledApiAvailable() throws an exception :", e.what());
    }
    catch (...)
    {
        Debug::Log("WindowsGraphicsCapture::IsCursorCaptureEnabledApiAvailable() throws an unknown exception.");
    }

    return isEnabled;
}


WindowsGraphicsCapture::WindowsGraphicsCapture(HWND hWnd)
    : hWnd_(hWnd)
    , hMonitor_(NULL)
{
    CreateItem();
}


WindowsGraphicsCapture::WindowsGraphicsCapture(HMONITOR hMonitor)
    : hMonitor_(hMonitor)
    , hWnd_(NULL)
{
    CreateItem();
}


void WindowsGraphicsCapture::CreateItem()
{
    if (!IsSupported()) return;

    try
    {
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
    }
    catch (...)
    {
        // Windows Graphics Capture is not supported with this HWND:
        // Debug::Log("WindowsGraphicsCapture::CreateItem() throws an exception.");
    }

    if (graphicsCaptureItem_)
    {
        graphicsCaptureItem_.DisplayName().c_str();
        size_ = graphicsCaptureItem_.Size();
    }
}


bool WindowsGraphicsCapture::CreatePoolAndSession()
{
    if (!graphicsCaptureItem_) return false;

    size_ = graphicsCaptureItem_.Size();

    const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager();
    auto &device = manager->GetDevice();

    try
    {
        std::scoped_lock lock(sessionAndPoolMutex_);

        captureFramePool_ = Direct3D11CaptureFramePool::CreateFreeThreaded(
            device, 
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            size_);
        graphicsCaptureSession_ = captureFramePool_.CreateCaptureSession(graphicsCaptureItem_);
        graphicsCaptureSession_.StartCapture();
    }
    catch (const std::exception &e)
    {
        Debug::Log("WindowsGraphicsCapture::CreatePoolAndSession() throws an exception :", e.what());
        return false;
    }
    catch (...)
    {
        Debug::Log("WindowsGraphicsCapture::CreatePoolAndSession() throws an unknown exception.");
        return false;
    }

    return true;
}


void WindowsGraphicsCapture::DestroyPoolAndSession()
{
    ReleaseLatestResult();

    std::scoped_lock lock(sessionAndPoolMutex_);

    try
    {
        if (captureFramePool_)
        {
            captureFramePool_.Close();
            captureFramePool_ = nullptr;
        }

        if (graphicsCaptureSession_)
        {
            graphicsCaptureSession_.Close();
            graphicsCaptureSession_ = nullptr;
        }
    }
    catch (const std::exception &e)
    {
        Debug::Log("WindowsGraphicsCapture::DestroyPoolAndSession() throws an exception :", e.what());
    }
    catch (...)
    {
        Debug::Log("WindowsGraphicsCapture::DestroyPoolAndSession() throws an unknown exception.");
    }
}


WindowsGraphicsCapture::~WindowsGraphicsCapture()
{
    Stop();
}


void WindowsGraphicsCapture::Start()
{
    if (isStarted_) return;

    if (CreatePoolAndSession())
    {
        isStarted_ = true;

        if (const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager())
        {
            manager->Add(shared_from_this());
        }
    }
}


void WindowsGraphicsCapture::RequestStop()
{
    hasStopRequested_ = true;
}


void WindowsGraphicsCapture::Stop()
{
    hasStopRequested_ = false;
    stopTimer_ = 0.f;

    if (!isStarted_) return;
    isStarted_ = false;

    DestroyPoolAndSession();

    if (const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager())
    {
        manager->Remove(shared_from_this());
    }
}


bool WindowsGraphicsCapture::ShouldStop() const
{
    constexpr float timer = 1.f;
    return stopTimer_ > timer || hasStopRequested_;
}


bool WindowsGraphicsCapture::IsAvailable() const
{
    return static_cast<bool>(graphicsCaptureItem_);
}


void WindowsGraphicsCapture::Update(float dt)
{
    if (!IsStarted()) return;

    stopTimer_ = stopTimer_ + dt;
}


void WindowsGraphicsCapture::EnableCursorCapture(bool enabled)
{
    if (isCursorCaptureEnabled_ == enabled) return;

    isCursorCaptureEnabled_ = enabled;

    if (!IsCursorCaptureEnabledApiSupported())
    {
        Debug::Log("CursorCaptureEnabled API is not available.");
        return;
    }

    std::scoped_lock lock(sessionAndPoolMutex_);

    if (graphicsCaptureSession_) 
    {
        graphicsCaptureSession_.IsCursorCaptureEnabled(enabled);
    }
}


WindowsGraphicsCapture::Result WindowsGraphicsCapture::TryGetLatestResult()
{
    UWC_SCOPE_TIMER(TryGetLatestResult)

    stopTimer_ = 0.f;

    ReleaseLatestResult();

    std::scoped_lock lock(sessionAndPoolMutex_);

    if (!captureFramePool_) return {};

    while (const auto nextFrame = captureFramePool_.TryGetNextFrame())
    {
        latestCaptureFrame_ = nextFrame;
    }
    if (!latestCaptureFrame_) return {};

    const auto surface = latestCaptureFrame_.Surface();
    if (!surface) return {};

    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    com_ptr<ID3D11Texture2D> texture;
    const auto hr = access->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void());
    if (FAILED(hr)) return {};

    const auto size = latestCaptureFrame_.ContentSize();
    const bool hasSizeChanged = 
        (size_.Width != size.Width) || 
        (size_.Height != size.Height);

    return { texture.get(), size.Width, size.Height, hasSizeChanged };
}


void WindowsGraphicsCapture::ReleaseLatestResult()
{
    std::scoped_lock lock(sessionAndPoolMutex_);

    if (latestCaptureFrame_)
    {
        latestCaptureFrame_ = nullptr;
    }
}


void WindowsGraphicsCapture::ChangePoolSize(int width, int height)
{
    std::scoped_lock lock(sessionAndPoolMutex_);

    if (!captureFramePool_) return;

    const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager();
    auto &device = manager->GetDevice();

    size_ = { width, height };
    captureFramePool_.Recreate(
        device, 
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        size_);
}


const wchar_t * WindowsGraphicsCapture::GetDisplayName() const
{
    static const wchar_t invalid[] = L"";
    if (!graphicsCaptureItem_) return invalid;
    if (graphicsCaptureItem_.DisplayName().empty()) return invalid;
    return graphicsCaptureItem_.DisplayName().c_str();
}


// ---


IDirect3DDevice & WindowsGraphicsCaptureManager::GetDevice()
{
    if (!deviceWinRt_)
    {
        const auto& uploader = WindowManager::GetUploadManager();
        while (!uploader->IsReady())
        {
            const std::chrono::microseconds waitTime(100);
            std::this_thread::sleep_for(waitTime);
        }

        if (auto device = uploader->GetDevice())
        {
            com_ptr<IDXGIDevice> dxgiDevice;
            const auto hr = device->QueryInterface<IDXGIDevice>(dxgiDevice.put());
            if (SUCCEEDED(hr))
            {
                com_ptr<::IInspectable> deviceWinRt;
                ::CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), deviceWinRt.put());
                deviceWinRt_ = deviceWinRt.as<IDirect3DDevice>();
            }
        }
    }

    return deviceWinRt_;
}


void WindowsGraphicsCaptureManager::Add(const std::shared_ptr<WindowsGraphicsCapture>& instance)
{
    std::scoped_lock lock(addedInstancesMutex_);
    addedInstances_.push_back(instance);
}


void WindowsGraphicsCaptureManager::Remove(const std::shared_ptr<WindowsGraphicsCapture>& instance)
{
    std::scoped_lock lock(removedInstancesMutex_);
    removedInstances_.push_back(instance);
}


void WindowsGraphicsCaptureManager::UpdateFromMainThread(float dt)
{
    std::scoped_lock instancesLock(instancesMutex_);

    for (const auto& instance : instances_)
    {
        if (instance && instance->IsValid())
        {
            instance->Update(dt);
        }
    }
}


void WindowsGraphicsCaptureManager::UpdateFromCaptureThread()
{
    UpdateAddInstances();

    {
        std::scoped_lock instancesLock(instancesMutex_);
        for (const auto& instance : instances_)
        {
            if (instance && instance->ShouldStop())
            {
                instance->Stop();
            }
        }
    }

    UpdateRemoveInstances();
}


void WindowsGraphicsCaptureManager::UpdateAddInstances()
{
    std::scoped_lock addedInstancesLock(addedInstancesMutex_);
    std::scoped_lock instancesLock(instancesMutex_);
    for (const auto& instance : addedInstances_)
    {
        instances_.push_back(instance);
    }
    addedInstances_.clear();
}


void WindowsGraphicsCaptureManager::UpdateRemoveInstances()
{
    std::scoped_lock removedInstancesLock(removedInstancesMutex_);
    std::scoped_lock instancesLock(instancesMutex_);
    for (const auto& instance : removedInstances_)
    {
        instances_.erase(
            std::remove(
                instances_.begin(),
                instances_.end(),
                instance),
            instances_.end());
    }

    instances_.erase(
        std::remove_if(
            instances_.begin(),
            instances_.end(),
            [](const auto &instance) { return !instance; }),
        instances_.end());

    removedInstances_.clear();
}


void WindowsGraphicsCaptureManager::StopAllInstances()
{
    UpdateAddInstances();

    {
        std::scoped_lock instancesLock(instancesMutex_);
        for (const auto& instance : instances_)
        {
            if (instance && instance->IsValid())
            {
                instance->Stop();
            }
        }
    }

    UpdateRemoveInstances();
}
