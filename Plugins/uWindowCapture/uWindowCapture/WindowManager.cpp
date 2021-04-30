#include <algorithm>
#include <oleacc.h>
#include "WindowManager.h"
#include "WindowTexture.h"
#include "Message.h"
#include "Util.h"
#include "Debug.h"

using namespace Microsoft::WRL;



UWC_SINGLETON_INSTANCE(WindowManager)


void WindowManager::Initialize()
{
    {
        UWC_SCOPE_TIMER(InitWindowsGraphicsCaptureManager);
        windowsGraphicsCaptureManager_ = std::make_unique<WindowsGraphicsCaptureManager>();
    }
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
    windowsGraphicsCaptureManager_.reset();
    cursor_.reset();
    windows_.clear();
}


void WindowManager::Update(float dt)
{
    if (windowsGraphicsCaptureManager_) 
    {
        windowsGraphicsCaptureManager_->Update(dt);
    }
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
        UpdateWindows();
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


const std::unique_ptr<WindowsGraphicsCaptureManager>& WindowManager::GetWindowsGraphicsCaptureManager()
{
    return WindowManager::Get().windowsGraphicsCaptureManager_;
}


const std::unique_ptr<Cursor>& WindowManager::GetCursor()
{
    return WindowManager::Get().cursor_;
}


bool WindowManager::CheckExistence(int id) const
{
    return windows_.find(id) != windows_.end();
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
            if (window->GetWindowHandle() == hWnd) 
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
            other->GetWindowHandle() == window->GetParentHandle() ||
            other->GetWindowHandle() == window->GetOwnerHandle()
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


std::shared_ptr<Window> WindowManager::FindOrAddWindow(const Window::Data1 &data)
{
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [&](const auto& pair) 
        { 
            const auto& window = pair.second;
            return data.isDesktop ?
                (window->IsDesktop() && window->GetMonitorHandle() == data.hMonitor) :
                (window->GetWindowHandle() == data.hWnd);
        });

    if (it != windows_.end())
    {
        return it->second;
    }

    const auto id = lastWindowId_++;
    auto window = std::make_shared<Window>(id, data);
    windows_.emplace(id, window);

    return window;
}


void WindowManager::UpdateWindows()
{
    UWC_SCOPE_TIMER(UpdateWindows);

    for (const auto& pair : windows_)
    {
        pair.second->isAlive_ = false;
    }

    {
        std::lock_guard<std::mutex> lock(windowsDataListMutex_);

        for (auto&& data1 : windowDataList_[0])
        {
            auto window = WindowManager::Get().FindOrAddWindow(data1);
            if (window)
            {
                window->SetData(data1);
                window->isAlive_ = true;

                // Newly added
                if (window->frameCount_ == 0)
                {
                    auto &data2 = window->data2_;
                    const auto hWnd = window->GetWindowHandle();

                    if (!window->IsDesktop())
                    {
                        data2.hParent = ::GetParent(hWnd);
                        data2.hInstance = reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
                        data2.threadId = ::GetWindowThreadProcessId(hWnd, &data2.processId);
                        data2.isAltTabWindow = IsAltTabWindow(hWnd);
                        GetWindowClassName(hWnd, data2.className);
                        data2.isApplicationFrameWindow = IsApplicationFrameWindow(data2.className);
                        data2.isUWP = data2.isApplicationFrameWindow || IsUWP(data2.processId);
                        window->UpdateTitle();
                        window->UpdateIsBackground();
                    }
                    else
                    {
                        data2.hParent = NULL;
                        data2.hInstance = NULL;
                        data2.threadId = ::GetWindowThreadProcessId(hWnd, &data2.processId);
                        data2.isAltTabWindow = false;
                        data2.isApplicationFrameWindow = false;
                        data2.isUWP = false;
                        data2.isBackground = false;
                        data2.className = "";
                        window->UpdateTitle();
                    }

                    if (auto parent = FindParentWindow(window))
                    {
                        window->parentId_ = parent->GetId();
                    }

                    window->InitIcon();

                    MessageManager::Get().Add({ MessageType::WindowAdded, window->GetId(), window->GetWindowHandle() });
                }
                else
                {
                    if (window->hasTitleUpdateRequested_ || window->GetTitle().empty()) 
                    {
                        window->hasTitleUpdateRequested_ = false;
                        window->UpdateTitle();
                    }
                    window->UpdateIsBackground();
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
            MessageManager::Get().Add({ MessageType::WindowRemoved, id, window->GetWindowHandle() });
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
    UWC_SCOPE_TIMER(UpdateWindowHandleList);

    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!::IsWindow(hWnd) || !::IsWindowVisible(hWnd) || ::IsHungAppWindow(hWnd))
        {
            return TRUE;
        }

        Window::Data1 data;
        data.hWnd = hWnd;
        data.hOwner = ::GetWindow(hWnd, GW_OWNER);
        ::GetWindowRect(hWnd, &data.windowRect);
        ::GetClientRect(hWnd, &data.clientRect);
        data.zOrder = ::GetWindowZOrder(hWnd);
        data.hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
        data.isDesktop = false;

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowDataList_[1].push_back(data);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);
    if (!::EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError(__FUNCTION__, "EnumWindows");
    }

    static const auto _EnumDisplayMonitorsCallback = [](HMONITOR hMonitor, HDC hDc, LPRECT lpRect, LPARAM lParam) -> BOOL
    {
        const auto hWnd = GetDesktopWindow();;

        Window::Data1 data;
        data.hWnd = hWnd;
        data.hOwner = NULL;
        data.windowRect = *lpRect;
        data.clientRect = *lpRect;
        data.zOrder = 0;
        data.hMonitor = hMonitor;
        data.isDesktop = true;

        auto thiz = reinterpret_cast<WindowManager*>(lParam);
        thiz->windowDataList_[1].push_back(data);

        return TRUE;
    };

    using EnumDisplayMonitorsCallbackType = BOOL(CALLBACK *)(HMONITOR, HDC, LPRECT, LPARAM);
    static const auto EnumDisplayMonitorsCallback = static_cast<EnumDisplayMonitorsCallbackType>(_EnumDisplayMonitorsCallback);
    if (!::EnumDisplayMonitors(NULL, NULL, EnumDisplayMonitorsCallback, reinterpret_cast<LPARAM>(this)))
    {
        OutputApiError(__FUNCTION__, "EnumDisplayMonitors");
    }

    std::sort(
        windowDataList_[1].begin(), 
        windowDataList_[1].end(), 
        [](const auto& a, const auto& b) 
        {
            return 
                a.hOwner == nullptr &&
                b.hOwner != nullptr;
        });

    {
        std::lock_guard<std::mutex> lock(windowsDataListMutex_);
        std::swap(windowDataList_[0], windowDataList_[1]);
    }
    windowDataList_[1].clear();

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