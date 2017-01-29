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
    {
        UWC_SCOPE_TIMER(InitUploadManager);
        uploadManager_ = std::make_unique<UploadManager>();
    }
    {
        UWC_SCOPE_TIMER(InitCaptureManager);
        captureManager_ = std::make_unique<CaptureManager>();
    }
    {
        UWC_SCOPE_TIMER(StartThread);
        StartWindowHandleListThread();
    }
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


std::shared_ptr<Window> WindowManager::GetWindowFromPoint(POINT point) const
{
    auto hWnd = ::WindowFromPoint(point);

    if (hWnd)
    {
        auto it = std::find_if(
            windows_.begin(),
            windows_.end(),
            [hWnd](const auto& pair) 
            { 
                return pair.second->GetHandle() == hWnd; 
            });

        if (it == windows_.end()) 
        {
            DWORD thread, process;
            thread = ::GetWindowThreadProcessId(hWnd, &process);

            it = std::find_if(
                windows_.begin(),
                windows_.end(),
                [=](const auto& pair) 
                { 
                    const auto& window = pair.second;
                    return 
                        window->GetOwnerHandle() == nullptr && 
                        window->GetProcessId() == process && 
                        window->GetThreadId() == thread;
                });
        }

        if (it != windows_.end()) 
        {
            return it->second;
        }
    }

    return nullptr;
}


std::shared_ptr<Window> WindowManager::GetCursorWindow() const
{
    return cursorWindow_.lock();
}


std::shared_ptr<Window> WindowManager::FindParentWindow(const std::shared_ptr<Window>& window) const
{
    const auto it = std::find_if(
        windows_.begin(), 
        windows_.end(), 
        [&window](const std::pair<int, std::shared_ptr<Window>>& pair) 
        {
            const auto& other = pair.second;

            if (other->GetId() == window->GetId()) 
            {
                return false;
            }

            if (other->GetHandle() == window->GetParentHandle()) 
            {
                return true;
            }

            if (other->GetHandle() == window->GetOwnerHandle())
            {
                return true;
            }

            if (other->GetParentId()  == -1 &&
                other->GetProcessId() == window->GetProcessId() &&
                other->GetThreadId()  == window->GetThreadId()) 
            {
                return true;
            }

            return false;
        });

    return (it != windows_.end()) ? it->second : nullptr;
}


std::shared_ptr<Window> WindowManager::FindOrAddWindow(HWND hWnd)
{
    const auto it = std::find_if(
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
        for (const auto& info : windowInfoList_[0])
        {
            if (auto window = WindowManager::Get().FindOrAddWindow(info.hWnd))
            {
                window->isAlive_ = true;
                window->hWndOwner_ = info.hOwner;
                window->hWndParent_ = info.hParent;
                window->instance_ = info.hInstance;
                window->processId_ = info.processId;
                window->threadId_ = info.threadId;
                window->rect_ = std::move(info.rect);
                window->zOrder_ = info.zOrder;
                window->title_ = info.title; 

                if (window->frameCount_ == 0)
                {
                    if (auto parent = FindParentWindow(window))
                    {
                        window->parentId_ = parent->GetId();
                    }
                }

                window->frameCount_++;
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
        if (!::IsWindow(hWnd) || !::IsWindowVisible(hWnd))
        {
            return TRUE;
        }

        WindowInfo info;
        info.hWnd = hWnd;
        info.hOwner = ::GetWindow(hWnd, GW_OWNER);
        info.hParent = ::GetParent(hWnd);
        info.hInstance = reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
        info.zOrder = ::GetWindowZOrder(hWnd);
        ::GetWindowRect(hWnd, &info.rect);
        info.threadId = ::GetWindowThreadProcessId(hWnd, &info.processId);

        auto title = info.title;
        const UINT timeout = 200 /* milliseconds */;
        if (::GetWindowTitle(hWnd, title, timeout))
        {
            info.title = title;
        }

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowInfoList_[1].push_back(info);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);
    if (!::EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError("EnumWindows");
    }

    std::sort(
        windowInfoList_[1].begin(), 
        windowInfoList_[1].end(), 
        [](const auto& a, const auto& b) 
        {
            return 
                a.hOwner == nullptr &&
                b.hOwner != nullptr;
        });

    {
        std::lock_guard<std::mutex> lock(windowsHandleListMutex_);
        std::swap(windowInfoList_[0], windowInfoList_[1]);
    }
    windowInfoList_[1].clear();

    POINT cursorPos;
    if (::GetCursorPos(&cursorPos))
    {
        cursorWindow_ = GetWindowFromPoint(cursorPos);
    }
}


void WindowManager::RenderWindows()
{
    for (auto&& pair : windows_)
    {
        pair.second->Render();
    }
}