#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>

#include "Singleton.h"
#include "Thread.h"
#include "CaptureManager.h"
#include "UploadManager.h"
#include "WindowsGraphicsCapture.h"
#include "Window.h"
#include "Cursor.h"


class WindowManager
{
    UWC_SINGLETON(WindowManager)

public:
    void Initialize();
    void Finalize();
    void Update(float dt);
    void Render();
    bool CheckExistence(int id) const;
    std::shared_ptr<Window> GetWindow(int id) const;
    std::shared_ptr<Window> GetWindowFromPoint(POINT point) const;
    std::shared_ptr<Window> GetCursorWindow() const;

    static const std::unique_ptr<CaptureManager>& GetCaptureManager();
    static const std::unique_ptr<UploadManager>& GetUploadManager();
    static const std::unique_ptr<WindowsGraphicsCaptureManager>& GetWindowsGraphicsCaptureManager();
    static const std::unique_ptr<Cursor>& GetCursor();

private:
    std::shared_ptr<Window> FindParentWindow(const std::shared_ptr<Window>& window) const;
    std::shared_ptr<Window> FindOrAddWindow(const Window::Data1 &data);

    void StartWindowHandleListThread();
    void StopWindowHandleListThread();
    void UpdateWindowHandleList();
    void UpdateWindows();
    void RenderWindows();

    std::unique_ptr<CaptureManager> captureManager_;
    std::unique_ptr<UploadManager> uploadManager_;
    std::unique_ptr<WindowsGraphicsCaptureManager> windowsGraphicsCaptureManager_;
    std::unique_ptr<Cursor> cursor_;

    std::map<int, std::shared_ptr<Window>> windows_;
    int lastWindowId_ = 0;
    std::weak_ptr<Window> cursorWindow_;
    mutable std::mutex windowsListMutex_;

    ThreadLoop windowHandleListThreadLoop_ = { L"uWindowCapture - Window Handle List Thread" };

    std::vector<Window::Data1> windowDataList_[2];
    mutable std::mutex windowsDataListMutex_;
};

