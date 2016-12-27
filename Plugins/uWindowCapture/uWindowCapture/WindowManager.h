#pragma once

#include <Windows.h>
#include <map>
#include <memory>

class Window;

class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	std::shared_ptr<Window> GetWindow(int id) const;
	int Add(HWND hwnd);
	void Remove(int id);

private:
	std::map<int, std::shared_ptr<Window>> windows_;
	int currentId_ = 0;
};

