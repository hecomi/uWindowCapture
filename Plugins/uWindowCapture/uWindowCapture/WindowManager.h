#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <memory>
#include <mutex>

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

    void StartWindowHandleListThread();
    void StopWindowHandleListThread();
    void UpdateWindowHandleList();

    void UpdateWindows();
    void RenderWindows();

    ThreadLoop windowHandleListThreadLoop_;
    int lastWindowId_ = 0;
    std::map<int, std::shared_ptr<Window>> windows_;

    struct WindowInfo
    {
        HWND hWnd;
        RECT rect;
        UINT zOrder;
    };
    std::vector<WindowInfo> windowHandleList_[2];
    mutable std::mutex windowsHandleListMutex_;
};

