#include <set>
#include "Message.h"



UWC_SINGLETON_INSTANCE(MessageManager)


UINT MessageManager::GetCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<UINT>(messages_.size());
}


const Message* MessageManager::GetHeadPointer() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (messages_.empty()) return nullptr;
    return &messages_[0];
}


void MessageManager::Add(Message message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    messages_.push_back(message);
}


void MessageManager::ClearAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    messages_.clear();
}


void MessageManager::ExcludeRemovedWindowEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::set<int> removedWinedowIds; 
    for (const auto& message : messages_)
    {
        if (message.type == MessageType::WindowRemoved)
        {
            removedWinedowIds.insert(message.windowId);
        }
    }

    for (auto it = messages_.begin(); it != messages_.end();)
    {
        const auto& message = *it;
        if (removedWinedowIds.find(message.windowId) != removedWinedowIds.end() && 
            message.type != MessageType::WindowRemoved)
        {
            it = messages_.erase(it);
            continue;
        }
        ++it;
    }
}
