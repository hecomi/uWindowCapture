#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>

#include "Singleton.h"
#include "Thread.h"


class Window;


enum class CapturePriority
{
    Immediate = 0,
    Queued = 1,
};


class WindowCaptureManager
{
public:
    void RequestCapture(int id, CapturePriority priority);
    void Update();
    void SetNumberPerFrame(UINT number);

private:
    void Enqueue(int id, bool back);
    int Dequeue();

    std::deque<int> queue_;
    UINT numberPerFrame_ = 1;
};


class WindowManager
{
    UWC_SINGLETON(WindowManager)

public:
    void Initialize();
    void Finalize();
    void Update();
    void Render();
    std::shared_ptr<Window> GetWindow(int id) const;
    const std::unique_ptr<WindowCaptureManager>& GetCaptureManager() const;

private:
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);

    void StartWindowHandleListThread();
    void StopWindowHandleListThread();
    void UpdateWindowHandleList();

    void UpdateWindows();
    void RenderWindows();

    int lastWindowId_ = 0;
    std::map<int, std::shared_ptr<Window>> windows_;
    ThreadLoop windowHandleListThreadLoop_;
    std::unique_ptr<WindowCaptureManager> captureManager_ = std::make_unique<WindowCaptureManager>();

    struct WindowInfo
    {
        HWND hWnd;
        RECT rect;
        UINT zOrder;
    };
    std::vector<WindowInfo> windowHandleList_[2];
    mutable std::mutex windowsHandleListMutex_;

};

