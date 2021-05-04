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


namespace
{


bool CallWinRtApiWithExceptionCheck(const std::function<void()> &func, const std::string& name)
{
    try
    {
        func();
    }
    catch (const winrt::hresult_error& e)
    {
        const int code = e.code();
        char buf[32];
        sprintf_s(buf, "0x%x", code);
        Debug::Log(name, " throws an exception: ", buf);
        return false;
    }
    catch (const std::exception& e)
    {
        Debug::Log(name, " throws an exception: ", e.what());
        return false;
    }
    catch (...)
    {
        Debug::Log(name, " throws an unknown exception.");
        return false;
    }

    return true;
}


}


// ---


bool WindowsGraphicsCapture::IsSupported()
{
    using ApiInfo = winrt::Windows::Foundation::Metadata::ApiInformation;

    static bool isChecked = false;
    static bool isAvailable = false;
    if (isChecked) return isAvailable;
    isChecked = true;

    if (!CallWinRtApiWithExceptionCheck([&]
    {
        isAvailable = ApiInfo::IsApiContractPresent(L"Windows.Foundation.UniversalApiContract", 8);
    }, "WindowsGraphicsCapture::IsSupported()"))
    {
        return false;
    }

    return isAvailable;
}


bool WindowsGraphicsCapture::IsCursorCaptureEnabledApiSupported()
{
    using ApiInfo = winrt::Windows::Foundation::Metadata::ApiInformation;

    if (!IsSupported()) return false;

    static bool isChecked = false;
    static bool isEnabled = false;
    if (isChecked) return isEnabled;
    isChecked = true;

    if (!CallWinRtApiWithExceptionCheck([&]
    {
        isEnabled = ApiInfo::IsPropertyPresent(
            L"Windows.Graphics.Capture.GraphicsCaptureSession",
            L"IsCursorCaptureEnabled");
    }, "WindowsGraphicsCapture::IsCursorCaptureEnabledApiAvailable()"))
    {
        return false;
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

    CallWinRtApiWithExceptionCheck([&]
    {
        const auto factory = get_activation_factory<GraphicsCaptureItem>();
        const auto interop = factory.as<IGraphicsCaptureItemInterop>();
        if (hWnd_)
        {
            interop->CreateForWindow(
                hWnd_,
                guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), 
                reinterpret_cast<void**>(put_abi(item_)));
        }
        else
        {
            interop->CreateForMonitor(
                hMonitor_,
                guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), 
                reinterpret_cast<void**>(put_abi(item_)));
        }
    }, "WindowsGraphicsCapture::CreateItem()");

    if (item_)
    {
        item_.DisplayName().c_str();
        size_ = item_.Size();
    }
}


bool WindowsGraphicsCapture::CreatePoolAndSession()
{
    if (!item_) return false;

    size_ = item_.Size();
    if (size_.Width == 0 || size_.Height == 0) return false;

    const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager();
    auto &device = manager->GetDevice();

    return CallWinRtApiWithExceptionCheck([&]
    {
        std::scoped_lock lock(sessionAndPoolMutex_);

        pool_ = Direct3D11CaptureFramePool::CreateFreeThreaded(
            device,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            size_);
        session_ = pool_.CreateCaptureSession(item_);
        session_.StartCapture();
    }, "WindowsGraphicsCapture::CreatePoolAndSession()");
}


void WindowsGraphicsCapture::DestroyPoolAndSession()
{
    ReleaseLatestResult();

    std::scoped_lock lock(sessionAndPoolMutex_);

    CallWinRtApiWithExceptionCheck([&]
    {
        if (pool_)
        {
            pool_.Close();
            pool_ = nullptr;
        }

        if (session_)
        {
            session_.Close();
            session_ = nullptr;
        }
    }, "WindowsGraphicsCapture::DestroyPoolAndSession()");
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
        latestFrameTime_ = std::chrono::high_resolution_clock::now();

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


void WindowsGraphicsCapture::Restart()
{
    Stop();
    CreateItem();
    Start();
    isSessionRestartRequested_ = false;
}


bool WindowsGraphicsCapture::IsAvailable() const
{
    return static_cast<bool>(item_);
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

    if (session_) 
    {
        session_.IsCursorCaptureEnabled(enabled);
    }
}


WindowsGraphicsCapture::Result WindowsGraphicsCapture::TryGetLatestResult()
{
    UWC_SCOPE_TIMER(TryGetLatestResult)

    stopTimer_ = 0.f;

    ReleaseLatestResult();

    std::scoped_lock lock(sessionAndPoolMutex_);

    if (!pool_) return {};

    while (const auto nextFrame = pool_.TryGetNextFrame())
    {
        frame_ = nextFrame;
    }

    if (frame_)
    {
        latestFrameTime_ = std::chrono::high_resolution_clock::now();
    }
    else
    {
        constexpr std::chrono::milliseconds timeoutThresh(1000);
        const auto currentTime = std::chrono::high_resolution_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - latestFrameTime_);
        if (dt > timeoutThresh)
        {
            isSessionRestartRequested_ = true;
        }
        return {};
    }

    const auto surface = frame_.Surface();
    if (!surface) return {};

    auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
    com_ptr<ID3D11Texture2D> texture;
    const auto hr = access->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void());
    if (FAILED(hr)) return {};

    const auto size = frame_.ContentSize();
    const bool hasSizeChanged = 
        (size_.Width != size.Width) || 
        (size_.Height != size.Height);

    return { texture.get(), size.Width, size.Height, hasSizeChanged };
}


void WindowsGraphicsCapture::ReleaseLatestResult()
{
    std::scoped_lock lock(sessionAndPoolMutex_);

    if (frame_)
    {
        frame_ = nullptr;
    }
}


void WindowsGraphicsCapture::ChangePoolSize(int width, int height)
{
    std::scoped_lock lock(sessionAndPoolMutex_);

    if (!pool_) return;

    const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager();
    auto &device = manager->GetDevice();

    size_ = { width, height };
    pool_.Recreate(
        device, 
        DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        size_);
}


const wchar_t * WindowsGraphicsCapture::GetDisplayName() const
{
    static const wchar_t invalid[] = L"";
    if (!item_) return invalid;
    if (item_.DisplayName().empty()) return invalid;
    return item_.DisplayName().c_str();
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
