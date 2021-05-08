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
        Debug::Error(name, " threw an exception: ", buf);
        return false;
    }
    catch (const std::exception& e)
    {
        Debug::Error(name, " threw an exception: ", e.what());
        return false;
    }
    catch (...)
    {
        Debug::Error(name, " threw an unknown exception.");
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


bool WindowsGraphicsCapture::CreateItem()
{
    if (!IsSupported()) return false;

    std::scoped_lock lock(itemMutex_);

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

    if (!item_) return false;

    size_ = item_.Size();

    return true;
}


bool WindowsGraphicsCapture::CreatePoolAndSession()
{
    std::scoped_lock lock(itemMutex_);

    if (!item_) return false;

    size_ = item_.Size();
    if (size_.Width == 0 || size_.Height == 0) return false;

    const auto& manager = WindowManager::GetWindowsGraphicsCaptureManager();
    if (!manager) return false;

    auto& device = manager->GetDevice();

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

    if (pool_)
    {
        CallWinRtApiWithExceptionCheck(
            [&] { pool_.Close(); }, 
            "WindowsGraphicsCapture::DestroyPoolAndSession() - Pool");
    }

    if (session_)
    {
        CallWinRtApiWithExceptionCheck(
            [&] { session_.Close(); }, 
            "WindowsGraphicsCapture::DestroyPoolAndSession() - Session");
    }

    pool_ = nullptr;
    session_ = nullptr;
}


WindowsGraphicsCapture::~WindowsGraphicsCapture()
{
}


void WindowsGraphicsCapture::RequestStart()
{
    isStartRequested_ = true;
}


void WindowsGraphicsCapture::Start()
{
    isStartRequested_ = false;
    stopTimer_ = 0.f;

    if (isStarted_) return;

    if (CreatePoolAndSession())
    {
        isStarted_ = true;
        restartTimer_ = 0.f;
    }
}


void WindowsGraphicsCapture::Stop()
{
    stopTimer_ = 0.f;

    if (!isStarted_) return;
    isStarted_ = false;

    DestroyPoolAndSession();
}


bool WindowsGraphicsCapture::ShouldStop() const
{
    constexpr float timer = 1.f;
    return stopTimer_ > timer;
}


bool WindowsGraphicsCapture::ShouldRestart() const
{
    return isRestartRequested_;
}


void WindowsGraphicsCapture::Restart()
{
    restartTimer_ = 0.f;
    isRestartRequested_ = false;

    Stop();
    if (!CreateItem()) return;
    Start();
}


bool WindowsGraphicsCapture::IsAvailable() const
{
    std::scoped_lock lock(itemMutex_);

    return item_ != nullptr;
}


void WindowsGraphicsCapture::Update(float dt)
{
    stopTimer_ = stopTimer_ + dt;
    restartTimer_ = restartTimer_ + dt;
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

    std::scoped_lock lock(sessionAndPoolMutex_);

    if (!pool_) return {};

    while (const auto nextFrame = pool_.TryGetNextFrame())
    {
        frame_ = nextFrame;
    }

    if (!frame_)
    {
        constexpr float timer = 1.f;
        if (restartTimer_ > timer)
        {
            isRestartRequested_ = true;
        }
        return {};
    };

    const auto surface = frame_.Surface();
    if (!surface) return {};

    restartTimer_ = 0.f;

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
    if (!manager) return;

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
    std::scoped_lock lock(itemMutex_);

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


std::shared_ptr<WindowsGraphicsCapture> WindowsGraphicsCaptureManager::Create(HWND hWnd)
{
    auto instance = std::make_shared<WindowsGraphicsCapture>(hWnd);
    if (!instance->IsAvailable()) return nullptr;

    std::scoped_lock lock(allInstancesMutex_);
    allInstances_.push_back(instance);

    return instance;
}


std::shared_ptr<WindowsGraphicsCapture> WindowsGraphicsCaptureManager::Create(HMONITOR hMonitor)
{
    auto instance = std::make_shared<WindowsGraphicsCapture>(hMonitor);
    if (!instance->IsAvailable()) return nullptr;

    std::scoped_lock lock(allInstancesMutex_);
    allInstances_.push_back(instance);

    return instance;
}


void WindowsGraphicsCaptureManager::Destroy(const std::shared_ptr<WindowsGraphicsCapture> &instance)
{
    std::scoped_lock lock(allInstancesMutex_);
    allInstances_.remove(instance);
}


void WindowsGraphicsCaptureManager::UpdateFromMainThread(float dt)
{
    std::scoped_lock lock(activeInstancesMutex_);

    for (const auto& instance : activeInstances_)
    {
        instance->Update(dt);
    }
}


void WindowsGraphicsCaptureManager::UpdateFromCaptureThread()
{
    StartInstances();
    RestartInstances();
    StopInstances();
}


void WindowsGraphicsCaptureManager::StartInstances()
{
    std::scoped_lock lock(allInstancesMutex_);

    for (const auto& instance : allInstances_)
    {
        if (instance->isStartRequested_)
        {
            instance->Start();

            if (instance->IsStarted())
            {
                std::scoped_lock lock(activeInstancesMutex_);
                activeInstances_.push_back(instance);
            }
        }
    }
}


void WindowsGraphicsCaptureManager::RestartInstances()
{
    std::scoped_lock lock(activeInstancesMutex_);

    for (const auto& instance : activeInstances_)
    {
        if (instance->ShouldRestart())
        {
            instance->Restart();
        }
    }
}


void WindowsGraphicsCaptureManager::StopInstances()
{
    std::scoped_lock lock(activeInstancesMutex_);

    for (auto it = activeInstances_.begin(); it != activeInstances_.end();)
    {
        auto instance = *it;
        if (instance->ShouldStop())
        {
            instance->Stop();
            it = activeInstances_.erase(it);
            continue;
        }
        ++it;
    }
}


void WindowsGraphicsCaptureManager::StopAllInstances()
{
    std::scoped_lock lock(activeInstancesMutex_);

    for (const auto& instance : activeInstances_)
    {
        instance->Stop();
    }
    activeInstances_.clear();
}
