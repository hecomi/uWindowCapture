#include <algorithm>
#include "WindowManager.h"
#include "Window.h"
#include "Thread.h"
#include "Message.h"
#include "Util.h"
#include "Debug.h"

using namespace Microsoft::WRL;



void WindowCaptureManager::RequestCapture(int id, CapturePriority priority)
{
    UWC_FUNCTION_SCOPE_TIMER
    auto window = WindowManager::Get().GetWindow(id);
    if (!window) return;

    switch (priority)
    {
        case CapturePriority::Immediate:
        {
            Enqueue(id, true);
            break;
        }
        case CapturePriority::Queued:
        {
            Enqueue(id, false);
            break;
        }
    }
}


void WindowCaptureManager::Enqueue(int id, bool back)
{
    UWC_FUNCTION_SCOPE_TIMER
    const auto it = std::find(queue_.begin(), queue_.end(), id);
    if (it == queue_.end())
    {
        if (back)
        {
            queue_.push_back(id);
        }
        else
        {
            queue_.push_front(id);
        }
    }
}


int WindowCaptureManager::Dequeue()
{
    UWC_FUNCTION_SCOPE_TIMER
    if (queue_.empty()) return -1;

    const auto id = queue_.back();
    queue_.pop_back();
    return id;
}


void WindowCaptureManager::SetNumberPerFrame(UINT number)
{
    UWC_FUNCTION_SCOPE_TIMER
    numberPerFrame_ = number;
}


void WindowCaptureManager::Update()
{
    for (UINT i = 0; i < numberPerFrame_; ++i)
    {
        const auto id = Dequeue();
        if (id < 0) break;

        if (auto window = WindowManager::Get().GetWindow(id))
        {
            window->RequestCapture();
        }
    }
}



// ---



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
    captureManager_->Update();
}


void WindowManager::Render()
{
    RenderWindows();
}


void WindowManager::StartWindowHandleListThread()
{
    windowHandleListThreadLoop_.Start([this]
    {
        UpdateWindowHandleList();
    });
}


void WindowManager::StopWindowHandleListThread()
{
    windowHandleListThreadLoop_.Stop();
}


const std::unique_ptr<WindowCaptureManager>& WindowManager::GetCaptureManager() const
{
    return captureManager_;
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
    UWC_SCOPE_TIMER(UpdateWindows)

    for (const auto& pair : windows_)
    {
        pair.second->isAlive_ = false;
    }

    {
        std::lock_guard<std::mutex> lock(windowsHandleListMutex_);
        for (const auto& info : windowHandleList_[0])
        {
            if (auto window = WindowManager::Get().FindOrAddWindow(info.hWnd))
            {
                window->isAlive_ = true;
                window->rect_ = info.rect;
                window->zOrder_ = info.zOrder;
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

        WindowInfo info;
        info.hWnd = hWnd;
        ::GetWindowRect(hWnd, &info.rect);
        info.zOrder = ::GetZOrder(hWnd);

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowHandleList_[1].push_back(info);

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