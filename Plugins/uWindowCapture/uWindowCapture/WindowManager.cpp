#include <algorithm>

#include "Common.h"
#include "WindowManager.h"
#include "Window.h"
#include "Uploader.h"
#include "Util.h"
#include "Thread.h"
#include "Message.h"
#include "Debug.h"

using namespace Microsoft::WRL;


UWC_SINGLETON_INSTANCE(WindowManager)


void WindowManager::Initialize()
{
    StartWindowHandleListThread();
}


void WindowManager::Finalize()
{
    StopWindowHandleListThread();
    windows_.clear();
}


void WindowManager::Update()
{
    UpdateWindows();
}


void WindowManager::Render()
{
    RenderWindows();
}


void WindowManager::StartWindowHandleListThread()
{
    windowHandleListThread_.Start([this]
    {
        UpdateWindowHandleList();
    });
}


void WindowManager::StopWindowHandleListThread()
{
    windowHandleListThread_.Stop();
}


std::shared_ptr<Window> WindowManager::GetWindow(int id) const
{
    auto it = windows_.find(id);
    if (it == windows_.end())
    {
        Debug::Error(__FUNCTION__, " => Window whose id is ", id, " does not exist.");
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

    const auto id = lastWindowId_++;
    auto window = std::make_shared<Window>(hWnd, id);
    windows_.emplace(id, window);

    MessageManager::Get().Add({ MessageType::WindowAdded, id, hWnd });

    return window;
}


void WindowManager::UpdateWindows()
{
    for (const auto& pair : windows_)
    {
        pair.second->isAlive_ = false;
    }

    {
        std::lock_guard<std::mutex> lock(windowsHandleListMutex_);
        for (HWND hWnd : windowHandleList_[0])
        {
            if (!::IsWindow(hWnd)) continue;

            if (auto window = WindowManager::Get().FindOrAddWindow(hWnd))
            {
                window->isAlive_ = true;
            }
        }
    }

    for (auto it = windows_.begin(); it != windows_.end();)
    {
        const auto id = it->first;
        auto& window = it->second;

        if (!window->isAlive_)
        {
            MessageManager::Get().Add({ MessageType::WindowRemoved, id, window->GetHandle() });
            windows_.erase(it++);
        }
        else
        {
            it++;
        }
    }
}


void WindowManager::UpdateWindowHandleList()
{
    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!::IsWindowVisible(hWnd) || !::IsWindow(hWnd) || !::IsWindowEnabled(hWnd))
        {
            return TRUE;
        }

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowHandleList_[1].push_back(hWnd);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);

    // add new windows and mark registered windows as alive
    if (!::EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError("EnumWindows");
    }

    {
        std::lock_guard<std::mutex> lock(windowsHandleListMutex_);
        windowHandleList_[0] = windowHandleList_[1];
        windowHandleList_[1].clear();
    }
}


void WindowManager::RenderWindows()
{
    for (auto&& pair : windows_)
    {
        pair.second->Render();
    }
}