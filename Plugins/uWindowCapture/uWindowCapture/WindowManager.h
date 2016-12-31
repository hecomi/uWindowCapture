#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <memory>

class Window;

enum class MessageType : int
{
    None = -1,
    WindowAdded = 0,
    WindowRemoved = 1,
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
    std::shared_ptr<Window> GetWindow(int id) const;
    UINT GetMessageCount() const;
    const Message* GetMessages() const;

private:
    std::shared_ptr<Window> FindOrAddWindow(HWND hwnd);
    void UpdateWindows();
    void AddMessage(Message message);
    void UpdateMessages();

    std::map<int, std::shared_ptr<Window>> windows_;
    std::vector<Message> messages_;
    int currentId_ = 0;
};

