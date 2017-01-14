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