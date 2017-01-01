#include <cassert>
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


void WindowManager::UpdateWindows()
{
    static const auto _EnumWindowsCallback = [](HWND hWnd, LPARAM lParam) -> BOOL
    {
        if (!::IsWindowVisible(hWnd) || !::IsWindow(hWnd))
        {
            return TRUE;
        }

        auto window = GetWindowManager()->FindOrAddWindow(hWnd);
        assert(window != nullptr);

        // set properties
        window->isAlive_ = true;
        window->owner_ = ::GetWindow(hWnd, GW_OWNER);
        window->isAltTabWindow_ = IsAltTabWindow(hWnd);

        // set title
        const auto titleLength = GetWindowTextLengthW(hWnd);
        std::vector<WCHAR> buf(titleLength + 1);
        if (!GetWindowTextW(hWnd, &buf[0], static_cast<int>(buf.size())))
        {
            OutputApiError("GetWindowTextW");
        }
        else
        {
            window->title_ = &buf[0];
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
