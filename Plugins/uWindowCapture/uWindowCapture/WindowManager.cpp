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


std::shared_ptr<Window> WindowManager::GetWindow(int id) const
{
	auto it = windows_.find(id);
	if (it == windows_.end())
	{
		Debug::Error("Window whose id is", id, " does not exist.");
		return nullptr;
	}
	return it->second;
}


const std::vector<WindowInfo>& WindowManager::GetWindowList() const
{
	return windowList_;
}


int WindowManager::Add(HWND hwnd)
{
	auto it = std::find_if(
		windows_.begin(),
		windows_.end(),
		[hwnd](const auto& pair) { return pair.second->GetHandle() == hwnd; });

	if (it != windows_.end())
	{
		Debug::Log("Given window handle has been already added.");
		return it->first;
	}

	const auto id = currentId_++;
	windows_.emplace(id, std::make_shared<Window>(hwnd));

	return id;
}


void WindowManager::Remove(int id)
{
	windows_.erase(id);
}


bool IsAltTabWindow(HWND hWnd)
{
    if (!IsWindowVisible(hWnd)) return false;

	// Ref: https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
	HWND hWndWalk = GetAncestor(hWnd, GA_ROOTOWNER);
	HWND hWndTry;
	while ((hWndTry = GetLastActivePopup(hWndWalk)) != hWndTry) {
		if (IsWindowVisible(hWndTry)) break;
		hWndWalk = hWndTry;
	}
	if (hWndWalk != hWnd)
	{
		return false;
	}

	// Tool window
	if (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
	{
        return false;
	}

	// Remove task tray programs
	TITLEBARINFO titleBar;
	titleBar.cbSize = sizeof(TITLEBARINFO);
	if (!GetTitleBarInfo(hWnd, &titleBar))
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


BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
	if (!IsAltTabWindow(hWnd)) return TRUE;

	auto& manager = GetWindowManager();

	WindowInfo info;
	info.handle = hWnd;
	if (!GetWindowTextW(hWnd, info.title, sizeof(info.title)))
	{
		return TRUE;
	}

	manager->AddWindowInfo(info);

	return TRUE;
}


void WindowManager::RequestUpdateList()
{
	windowList_.clear();

	if (!EnumWindows(EnumWindowsCallback, 0))
	{
		OutputApiError("EnumWindows");
	}
}


void WindowManager::AddWindowInfo(const WindowInfo& info)
{
	windowList_.push_back(info);
}