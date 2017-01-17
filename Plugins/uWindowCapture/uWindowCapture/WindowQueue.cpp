#include <algorithm>
#include "WindowQueue.h"



void WindowQueue::Enqueue(int id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = std::find(queue_.begin(), queue_.end(), id);
    if (it == queue_.end())
    {
        queue_.push_front(id);
    }
}


int WindowQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty()) return -1;

    const auto id = queue_.back();
    queue_.pop_back();
    return id;
}
