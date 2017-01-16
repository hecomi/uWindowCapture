#include <algorithm>
#include "WindowManager.h"
#include "Window.h"
#include "Message.h"
#include "Util.h"
#include "Debug.h"

using namespace Microsoft::WRL;



UWC_SINGLETON_INSTANCE(WindowManager)


void WindowManager::Initialize()
{
    uploadManager_ = std::make_unique<UploadManager>();
    captureManager_ = std::make_unique<CaptureManager>();
    StartWindowHandleListThread();
}


void WindowManager::Finalize()
{
    StopWindowHandleListThread();
    captureManager_.reset();
    uploadManager_.reset();
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
    windowHandleListThreadLoop_.Start([this]
    {
        UpdateWindowHandleList();
    });
}


void WindowManager::StopWindowHandleListThread()
{
    windowHandleListThreadLoop_.Stop();
}


const std::unique_ptr<CaptureManager>& WindowManager::GetCaptureManager()
{
    return WindowManager::Get().captureManager_;
}


const std::unique_ptr<UploadManager>& WindowManager::GetUploadManager()
{
    return WindowManager::Get().uploadManager_;
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
                window->owner_ = info.hOwner;
                window->processId_ = info.processId;
                window->cachedRect_ = std::move(info.rect);
                window->cachedZOrder_ = info.zOrder;
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
        if (!::IsWindow(hWnd) || !::IsWindowVisible(hWnd) || !::IsWindowEnabled(hWnd))
        {
            return TRUE;
        }

        WindowInfo info;
        info.hWnd = hWnd;
        info.hOwner = ::GetWindow(hWnd, GW_OWNER);
        info.zOrder = ::GetWindowZOrder(hWnd);
        ::GetWindowRect(hWnd, &info.rect);
        ::GetWindowThreadProcessId(hWnd, &info.processId);

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowHandleList_[1].push_back(info);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);
    if (!::EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError("EnumWindows");
    }

    {
        std::lock_guard<std::mutex> lock(windowsHandleListMutex_);
        std::swap(windowHandleList_[0], windowHandleList_[1]);
    }
    windowHandleList_[1].clear();
}


void WindowManager::RenderWindows()
{
    for (auto&& pair : windows_)
    {
        pair.second->Render();
    }
}