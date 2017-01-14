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
    void RequestUploadInBackgroundThread(int id);

private:
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);

    void StartWindowThread();
    void StopWindowThread();
    void UpdateWindows();
    void RenderWindows();

    void StartUploadThread();
    void StopUploadThread();
    void UploadTextures();

    std::map<int, std::shared_ptr<Window>> windows_;
    int lastWindowId_ = 0;
    mutable std::mutex windowsMutex_;

    Thread windowThread_;

    std::shared_ptr<class IsolatedD3D11Device> uploadDevice_;
    Thread uploadThread_;
    std::set<int> uploadList_;
    mutable std::mutex uploadMutex_;
};

