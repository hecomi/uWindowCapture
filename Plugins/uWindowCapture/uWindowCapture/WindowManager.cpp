#include <algorithm>

#include "Common.h"
#include "WindowManager.h"
#include "Window.h"
#include "Device.h"
#include "Util.h"
#include "Thread.h"
#include "Message.h"
#include "Debug.h"

using namespace Microsoft::WRL;


UWC_SINGLETON_INSTANCE(WindowManager)


void WindowManager::Initialize()
{
    uploadDevice_ = std::make_shared<IsolatedD3D11Device>();

    StartUploadThread();
    StartWindowThread();
}


void WindowManager::Finalize()
{
    StopWindowThread();
    StopUploadThread();
    {
        std::lock_guard<std::mutex> lock(windowsMutex_);
        windows_.clear();
    }
}


void WindowManager::Update()
{
    UpdateWindows();
}


void WindowManager::Render()
{
    RenderWindows();
}


void WindowManager::StartWindowThread()
{
    windowThread_.Start([this]
    {
        //UpdateWindows();
    });
}


void WindowManager::StopWindowThread()
{
    windowThread_.Stop();
}


std::shared_ptr<Window> WindowManager::GetWindow(int id) const
{
    std::lock_guard<std::mutex> lock(windowsMutex_);

    auto it = windows_.find(id);
    if (it == windows_.end())
    {
        Debug::Error("WindowManager::GetWindow() => Window whose id is ", id, " does not exist.");
        return nullptr;
    }
    return it->second;
}


std::shared_ptr<Window> WindowManager::FindOrAddWindow(HWND hWnd)
{
    std::lock_guard<std::mutex> lock(windowsMutex_);

    auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hWnd](const auto& pair) { return pair.second->GetHandle() == hWnd; });

    if (it != windows_.end())
    {
        return it->second;
    }

    const auto id = lastWindowId_++;
    auto window = std::make_shared<Window>(hWnd, id);
    windows_.emplace(id, window);

    MessageManager::Get().Add({ MessageType::WindowAdded, id, hWnd });

    return window;
}


void WindowManager::UpdateWindows()
{
    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!::IsWindowVisible(hWnd) || !::IsWindow(hWnd))
        {
            return TRUE;
        }

        if (auto window = WindowManager::Get().FindOrAddWindow(hWnd))
        {
            window->Update();
        }

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);

    // mark all window as inactive
    {
        std::lock_guard<std::mutex> lock(windowsMutex_);
        for (const auto& pair : windows_)
        {
            pair.second->isAlive_ = false;
        }
    }

    // add desktop
    if (auto desktop = FindOrAddWindow(GetDesktopWindow()))
    {
        desktop->title_ = L"Desktop";
        desktop->isAlive_ = true;
        desktop->isDesktop_ = true;

        // desktop image can be get through BitBlt.
        desktop->SetCaptureMode(Window::CaptureMode::BitBlt);
    }

    // add new windows and mark registered windows as alive
    if (!::EnumWindows(EnumWindowsCallback, 0))
    {
        OutputApiError("EnumWindows");
    }

    // remove inactive windows
    {
        std::lock_guard<std::mutex> lock(windowsMutex_);
        for (auto it = windows_.begin(); it != windows_.end();)
        {
            if (!it->second->isAlive_)
            {
                MessageManager::Get().Add({ MessageType::WindowRemoved, it->first, it->second->GetHandle() });
                windows_.erase(it++);
            }
            else
            {
                it++;
            }
        }
    }
}


void WindowManager::StartUploadThread()
{
    if (!uploadDevice_)
    {
        Debug::Error("WindowManager::StartUploadThread() => device is null.");
        return;
    }

    uploadThread_.Start([this] 
    { 
        UploadTextures(); 
    });
}


void WindowManager::StopUploadThread()
{
    uploadThread_.Stop();
}


void WindowManager::RequestUploadInBackgroundThread(int id)
{
    std::lock_guard<std::mutex> lock(uploadMutex_);

    auto window = GetWindow(id);
    if (!window) return;

    uploadList_.insert(id);
}


void WindowManager::UploadTextures()
{
    std::lock_guard<std::mutex> lock(uploadMutex_);

    for (const auto id : uploadList_)
    {
        if (auto window = GetWindow(id))
        {
            if (!window->IsWindow() || !window->IsVisible()) continue;
            window->UploadTextureToGpu(uploadDevice_);
        }
    }

    uploadList_.clear();
}


void WindowManager::RenderWindows()
{
    std::lock_guard<std::mutex> lock(windowsMutex_);

    for (auto&& pair : windows_)
    {
        pair.second->Render();
    }
}