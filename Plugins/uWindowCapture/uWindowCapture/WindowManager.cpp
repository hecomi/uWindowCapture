#include <algorithm>
#include "WindowManager.h"
#include "Window.h"
#include "Debug.h"



WindowManager::WindowManager()
{
}


WindowManager::~WindowManager()
{
}


void WindowManager::Update()
{
    UpdateMessages();
    UpdateWindows();
}


std::shared_ptr<Window> WindowManager::GetWindow(int id) const
{
    auto it = windows_.find(id);
    if (it == windows_.end())
    {
        Debug::Error("Window whose id is ", id, " does not exist.");
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
    auto window = std::make_shared<Window>(hWnd);
    windows_.emplace(id, window);

    Message msg;
    msg.type = MessageType::WindowAdded;
    msg.windowId = id;
    msg.windowHandle = hWnd;
    AddMessage(msg);

    return window;
}


UINT WindowManager::GetMessageCount() const
{
    return static_cast<UINT>(messages_.size());
}


const Message* WindowManager::GetMessages() const
{
    return &messages_[0];
}


void WindowManager::AddMessage(Message message)
{
    messages_.push_back(message);
}


void WindowManager::UpdateMessages()
{
    messages_.clear();
}


bool IsAltTabWindow(HWND hWnd)
{
    if (!::IsWindowVisible(hWnd)) return false;

    // Ref: https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
    HWND hWndWalk = ::GetAncestor(hWnd, GA_ROOTOWNER);
    HWND hWndTry;
    while ((hWndTry = ::GetLastActivePopup(hWndWalk)) != hWndTry) {
        if (::IsWindowVisible(hWndTry)) break;
        hWndWalk = hWndTry;
    }
    if (hWndWalk != hWnd)
    {
        return false;
    }

    // Tool window
    if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
    {
        return false;
    }

    // Remove task tray programs
    TITLEBARINFO titleBar;
    titleBar.cbSize = sizeof(TITLEBARINFO);
    if (!::GetTitleBarInfo(hWnd, &titleBar))
    {
        OutputApiError("GetTitleBarInfo");
        return false;
    }
    if (titleBar.rgstate[0] & STATE_SYSTEM_INVISIBLE)
    {
        return false;
    }

    return true;
}


void WindowManager::UpdateWindows()
{
    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!(IsAltTabWindow(hWnd) || ::GetWindow(hWnd, GW_HWNDFIRST) == hWnd)) return TRUE;

        auto window = GetWindowManager()->FindOrAddWindow(hWnd);
        window->SetAlive(true);

        const auto titleLength = GetWindowTextLengthW(hWnd);
        std::vector<WCHAR> buf(titleLength + 1);
        if (!GetWindowTextW(hWnd, &buf[0], static_cast<int>(buf.size())))
        {
            OutputApiError("GetWindowTextW");
        }
        else
        {
            window->SetTitle(&buf[0]);
        }

        return TRUE;
    };

    static const auto _EnumChildWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        return TRUE;

        if (!::IsWindowVisible(hWnd)) return TRUE;

        auto window = GetWindowManager()->FindOrAddWindow(hWnd);
        window->SetAlive(true);

        auto ptr = reinterpret_cast<std::shared_ptr<Window>*>(lParam);
        window->SetParent(*ptr);

        return TRUE;
    };

    using EnumWindowsCallbackType = BOOL(CALLBACK *)(HWND, LPARAM);
    static const auto EnumWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumWindowsCallback);
    static const auto EnumChildWindowsCallback = static_cast<EnumWindowsCallbackType>(_EnumChildWindowsCallback);

    for (const auto& pair : windows_)
    {
        pair.second->SetAlive(false);
    }

    if (auto desktop = FindOrAddWindow(GetDesktopWindow()))
    {
        desktop->SetTitle(L"Desktop");
        desktop->SetAlive(true);
    }

    if (!EnumWindows(EnumWindowsCallback, 0))
    {
        OutputApiError("EnumWindows");
    }

    for (const auto& pair : windows_)
    {
        const auto hParentWnd = pair.second->GetHandle();
        if (hParentWnd == GetDesktopWindow()) continue;
        const auto lParam = reinterpret_cast<LPARAM>(&pair.second);
        if (!EnumChildWindows(hParentWnd, EnumChildWindowsCallback, lParam))
        {
            OutputApiError("EnumChildWindows");
        }
    }

    for (auto it = windows_.begin(); it != windows_.end();)
    {
        if (!it->second->IsAlive())
        {
            Message msg;
            msg.type = MessageType::WindowRemoved;
            msg.windowId = it->first;
            msg.windowHandle = it->second->GetHandle();
            AddMessage(msg);

            windows_.erase(it++);
        }
        else
        {
            it++;
        }
    }
}
