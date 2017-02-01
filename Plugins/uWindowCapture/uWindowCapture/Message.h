#pragma once

#include <Windows.h>
#include <vector>
#include <mutex>

#include "Singleton.h"



enum class MessageType : int
{
    None = -1,
    WindowAdded = 0,
    WindowRemoved = 1,
    WindowCaptured = 2,
    WindowSizeChanged = 3,
    IconCaptured = 4,
    CursorCaptured = 4,
};


struct Message
{
    MessageType type = MessageType::None;
    int windowId = -1;
    void* userData = nullptr;
    Message(MessageType type, int id, void* userData)
        : type(type), windowId(id), userData(userData) {}
};


class MessageManager
{
    UWC_SINGLETON(MessageManager)

public:
    void Add(Message message);
    void ClearAll();
    UINT GetCount() const;
    const Message* GetHeadPointer() const;

private:
    std::vector<Message> messages_;
    mutable std::mutex mutex_;
};