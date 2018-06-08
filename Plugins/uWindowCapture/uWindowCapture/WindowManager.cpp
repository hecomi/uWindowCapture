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
        UWC_SCOPE_TIMER(Cursor);
        cursor_ = std::make_unique<Cursor>();
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
    cursor_.reset();
    windows_.clear();
}


void WindowManager::Update()
{
    UpdateWindows();
}


void WindowManager::Render()
{
    RenderWindows();
    cursor_->Render();
}


void WindowManager::StartWindowHandleListThread()
{
    windowHandleListThreadLoop_.Start([this]
    {
        UpdateWindowHandleList();
    }, std::chrono::milliseconds(16));
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


const std::unique_ptr<Cursor>& WindowManager::GetCursor()
{
    return WindowManager::Get().cursor_;
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

    while (hWnd != NULL)
    {
        DWORD thread, process;
        thread = ::GetWindowThreadProcessId(hWnd, &process);

        std::shared_ptr<Window> parent;
        int maxZOrder = INT_MAX;

        for (const auto& pair : windows_)
        {
            const auto& window = pair.second;

            // Return the window whose handle is same
            if (window->GetHandle() == hWnd) 
            {
                return window;
            }

            // Keep windows whose process and thread ids are same
            if ((window->GetThreadId() == thread) &&
                (window->GetProcessId() == process))
            {
                const int zOrder = window->GetZOrder();
                if (zOrder > maxZOrder)
                {
                    maxZOrder = zOrder;
                    parent = window;
                }
            }
        }

        // Return parent if found.
        if (parent)
        {
            return parent;
        }

        // Move next
        hWnd = ::GetAncestor(hWnd, GA_PARENT);
    }

    return nullptr;
}


std::shared_ptr<Window> WindowManager::GetCursorWindow() const
{
    return cursorWindow_.lock();
}


std::shared_ptr<Window> WindowManager::FindParentWindow(const std::shared_ptr<Window>& window) const
{
    std::shared_ptr<Window> parent = nullptr;
    int minDeltaZOrder = INT_MAX;
    int selfZOrder = window->GetZOrder();

    for (const auto& pair : windows_)
    {
        const auto& other = pair.second;

        if (other->GetId() == window->GetId()) 
        {
            continue;
        }

        if ((
            other->GetHandle() == window->GetParentHandle() ||
            other->GetHandle() == window->GetOwnerHandle()
        ) || 
        (
            ((other->GetParentId()  == -1 || other->IsAltTab()) &&
            other->GetProcessId() == window->GetProcessId() &&
            other->GetThreadId()  == window->GetThreadId()) 
        ))
        {
            // TODO: This is not accurate, should find the correct way to detect the parent.
            const int zOrder = other->GetZOrder();
            const int deltaZOrder = zOrder - selfZOrder;
            if (deltaZOrder > 0 && deltaZOrder < minDeltaZOrder)
            {
                minDeltaZOrder = deltaZOrder;
                parent = other;
            }
        }
    }

    return parent;
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


std::shared_ptr<Window> WindowManager::FindOrAddDesktop(UINT displayId)
{
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [displayId](const auto& pair) { return pair.second->displayId_ == displayId; });

    if (it != windows_.end())
    {
        return it->second;
    }

    const auto id = lastWindowId_++;
    auto window = std::make_shared<Window>(::GetDesktopWindow(), id);
    windows_.emplace(id, window);

    MessageManager::Get().Add({ MessageType::WindowAdded, id, window->hWnd_ });

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
            auto window = info.isDesktop ?
                WindowManager::Get().FindOrAddDesktop(info.displayId) :
                WindowManager::Get().FindOrAddWindow(info.hWnd);
            if (window)
            {
                window->isAlive_ = true;
                window->hWndOwner_ = info.hOwner;
                window->hWndParent_ = info.hParent;
                window->instance_ = info.hInstance;
                window->processId_ = info.processId;
                window->threadId_ = info.threadId;
                window->windowRect_ = std::move(info.windowRect);
                window->clientRect_ = std::move(info.clientRect);
                window->zOrder_ = info.zOrder;
                window->title_ = std::move(info.title); 
                window->displayId_ = info.displayId;
                window->captureArea_ = info.captureArea;

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
        if (!::IsWindow(hWnd) || !::IsWindowVisible(hWnd) || ::IsHungAppWindow(hWnd))
        {
            return TRUE;
        }

        WindowInfo info;
        info.hWnd = hWnd;
        info.hOwner = ::GetWindow(hWnd, GW_OWNER);
        info.hParent = ::GetParent(hWnd);
        info.hInstance = reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
        info.zOrder = ::GetWindowZOrder(hWnd);
        ::GetWindowRect(hWnd, &info.windowRect);
        ::GetClientRect(hWnd, &info.clientRect);
        info.threadId = ::GetWindowThreadProcessId(hWnd, &info.processId);
        info.isDesktop = false;
        info.displayId = -1;
        info.captureArea = { 0, 0, 0, 0 };

        const UINT timeout = 16 /* milliseconds */;
        GetWindowTitle(hWnd, info.title, timeout);

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowInfoList_[1].push_back(info);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);
    if (!::EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError(__FUNCTION__, "EnumWindows");
    }

    struct DesktopTmpInfo
    {
        std::vector<WindowInfo> windowList;
        LONG left = LONG_MAX;
        LONG top = LONG_MAX;
        UINT id = 0;
    } desktopTmpInfo;

    static const auto _EnumDisplayMonitorsCallback = [](HMONITOR hMonitor, HDC hDc, LPRECT lpRect, LPARAM lParam) -> BOOL
    {
        const auto hWnd = GetDesktopWindow();;
        auto tmp = reinterpret_cast<DesktopTmpInfo*>(lParam);

        WindowInfo info;
        info.hWnd = hWnd;
        info.hOwner = ::GetWindow(hWnd, GW_OWNER);
        info.hParent = ::GetParent(hWnd);
        info.hInstance = reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
        info.zOrder = ::GetWindowZOrder(hWnd);
        info.windowRect = *lpRect;
        info.clientRect = *lpRect;
        info.threadId = ::GetWindowThreadProcessId(hWnd, &info.processId);
        info.title = std::wstring(L"Desktop") + std::to_wstring(tmp->id);
        info.isDesktop = true;
        info.displayId = tmp->id;

        tmp->windowList.push_back(info);
        tmp->left = min(tmp->left, lpRect->left);
        tmp->top = min(tmp->top, lpRect->top);
        tmp->id++;

        return TRUE;
    };

    using EnumDisplayMonitorsCallbackType = BOOL(CALLBACK *)(HMONITOR, HDC, LPRECT, LPARAM);
    static const auto EnumDisplayMonitorsCallback = static_cast<EnumDisplayMonitorsCallbackType>(_EnumDisplayMonitorsCallback);
    if (!::EnumDisplayMonitors(NULL, NULL, EnumDisplayMonitorsCallback, reinterpret_cast<LPARAM>(&desktopTmpInfo)))
    {
        OutputApiError(__FUNCTION__, "EnumDisplayMonitors");
    }

    for (auto &&window : desktopTmpInfo.windowList)
    {
        window.captureArea = window.windowRect;
        window.captureArea.left -= desktopTmpInfo.left;
        window.captureArea.right -= desktopTmpInfo.left;
        window.captureArea.top -= desktopTmpInfo.top;
        window.captureArea.bottom -= desktopTmpInfo.top;

        windowInfoList_[1].push_back(window);
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