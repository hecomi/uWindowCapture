#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <memory>
#include <mutex>

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
};

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();
    void Update();
    void AddToUploadList(int id);
    void Render();
    std::shared_ptr<Window> GetWindow(int id) const;
    void AddMessage(const Message& message);
    UINT GetMessageCount() const;
    const Message* GetMessages() const;

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
    std::vector<Message> messages_;
    int currentId_ = 0;
    mutable std::mutex messageMutex_;

    std::shared_ptr<class IsolatedD3D11Device> uploadDevice_;
    std::thread uploadThread_;
    std::mutex mutex_;
    std::vector<int> uploadList_;
    bool isUploadThreadRunning_ = false;
};

