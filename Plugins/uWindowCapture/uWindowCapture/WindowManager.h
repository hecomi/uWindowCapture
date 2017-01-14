#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <mutex>
#include <atomic>

#include "Singleton.h"
#include "Thread.h"


class Window;


class WindowManager
{
    UWC_SINGLETON(WindowManager)

public:
    void Initialize();
    void Finalize();
    void Update();
    void Render();
    std::shared_ptr<Window> GetWindow(int id) const;

private:
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);

    void StartWindowThread();
    void StopWindowThread();
    void UpdateWindows();
    void RenderWindows();

    Thread windowThread_;
    int lastWindowId_ = 0;
    std::map<int, std::shared_ptr<Window>> windows_;
    mutable std::mutex windowsMutex_;
};

