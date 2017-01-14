#include <algorithm>

#include "Common.h"
#include "WindowManager.h"
#include "Window.h"
#include "Device.h"
#include "Util.h"
#include "Debug.h"

using namespace Microsoft::WRL;



WindowManager::WindowManager()
{
    InitializeDevice();
    StartUploadThread();
}


WindowManager::~WindowManager()
{
    windows_.clear();
    StopUploadThread();
}


void WindowManager::Update()
{
    UpdateMessages();
    UpdateWindows();
}


void WindowManager::Render()
{
    RenderWindows();
}


std::shared_ptr<Window> WindowManager::GetWindow(int id) const
{
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
    auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hWnd](const auto& pair) { return pair.second->GetHandle() == hWnd; });

    if (it != windows_.end())
    {
        return it->second;
    }

    const auto id = currentId_++;
    auto window = std::make_shared<Window>(hWnd, id);
    windows_.emplace(id, window);
    AddMessage({ MessageType::WindowAdded, id, hWnd });

    return window;
}


UINT WindowManager::GetMessageCount() const
{
    std::lock_guard<std::mutex> lock(messageMutex_);
    return static_cast<UINT>(messages_.size());
}


const Message* WindowManager::GetMessages() const
{
    std::lock_guard<std::mutex> lock(messageMutex_);
    if (messages_.empty()) return nullptr;
    return &messages_[0];
}


void WindowManager::AddMessage(Message message)
{
    std::lock_guard<std::mutex> lock(messageMutex_);
    messages_.push_back(message);
}


void WindowManager::UpdateMessages()
{
    std::lock_guard<std::mutex> lock(messageMutex_);
    messages_.clear();
}


void WindowManager::UpdateWindows()
{
    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!::IsWindowVisible(hWnd) || !::IsWindow(hWnd))
        {
            return TRUE;
        }

        if (auto window = GetWindowManager()->FindOrAddWindow(hWnd))
        {
            window->Update();
        }

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);

    // mark all window as inactive
    for (const auto& pair : windows_)
    {
        pair.second->isAlive_ = false;
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
    for (auto it = windows_.begin(); it != windows_.end();)
    {
        if (!it->second->isAlive_)
        {
            AddMessage({ MessageType::WindowRemoved, it->first, it->second->GetHandle() });
            windows_.erase(it++);
        }
        else
        {
            it++;
        }
    }
}


void WindowManager::InitializeDevice()
{
    uploadDevice_ = std::make_shared<IsolatedD3D11Device>();
}


void WindowManager::StartUploadThread()
{
    if (!uploadDevice_)
    {
        Debug::Error("WindowManager::StartUploadThread() => device is null.");
        return;
    }

    if (isUploadThreadRunning_) return;
    isUploadThreadRunning_ = true;

    uploadThread_ = std::thread([this] 
    {
        while (isUploadThreadRunning_)
        {
            ScopedThreadSleeper(std::chrono::microseconds(1000000 / 60));
            UploadTextures();
        }
    });
}


void WindowManager::StopUploadThread()
{
    if (!isUploadThreadRunning_) return;

    isUploadThreadRunning_ = false;
    if (uploadThread_.joinable())
    {
        uploadThread_.join();
    }
}


void WindowManager::RequestUploadInBackgroundThread(int id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto window = GetWindow(id);
    if (!window) return;

    // Skip if id is already registered to queue.
    auto it = std::find(uploadList_.begin(), uploadList_.end(), id);
    if (it != uploadList_.end()) return;

    uploadList_.push_back(id);
}


void WindowManager::UploadTextures()
{
    std::lock_guard<std::mutex> lock(mutex_);

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
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto&& pair : windows_)
    {
        pair.second->Render();
    }
}