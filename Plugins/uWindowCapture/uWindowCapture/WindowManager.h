#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <memory>

class Window;

struct WindowInfo
{
	HWND handle;
	WCHAR title[256];
};

class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	std::shared_ptr<Window> GetWindow(int id) const;
	const std::vector<WindowInfo>& GetWindowList() const;
	int Add(HWND hwnd);
	void Remove(int id);
	void RequestUpdateList();
	void AddWindowInfo(const WindowInfo& info);

private:
	std::map<int, std::shared_ptr<Window>> windows_;
	std::vector<WindowInfo> windowList_;
	int currentId_ = 0;
};

