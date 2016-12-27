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