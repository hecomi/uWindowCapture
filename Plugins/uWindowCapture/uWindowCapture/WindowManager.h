#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <mutex>
#include <atomic>

#include "Thread.h"


class Window;


enum class MessageType : int
{
    None = -1,
    WindowAdded = 0,
    WindowRemoved = 1,
    WindowCaptured = 2,
    WindowSizeChanged = 3,
};


struct Message
{
    MessageType type = MessageType::None;
    int windowId = -1;
    HWND windowHandle = nullptr;
    Message(MessageType type, int id, HWND handle)
        : type(type), windowId(id), windowHandle(handle) {}
};


class WindowManager
{
public:
    WindowManager();
    ~WindowManager();
    void Update();
    void Render();
    std::shared_ptr<Window> GetWindow(int id) const;
    void AddMessage(Message message);
    UINT GetMessageCount() const;
    const Message* GetMessages() const;
    void RequestUploadInBackgroundThread(int id);

private:
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);
    void UpdateWindows();
    void UpdateMessages();
    void RenderWindows();

    void InitializeDevice();
    void StartUploadThread();
    void StopUploadThread();
    void UploadTextures();

    std::map<int, std::shared_ptr<Window>> windows_;
    std::mutex windowsMutex_;

    std::vector<Message> messages_;
    int currentId_ = 0;
    mutable std::mutex messageMutex_;

    std::shared_ptr<class IsolatedD3D11Device> uploadDevice_;
    Thread uploadThread_;
    std::set<int> uploadList_;
};

