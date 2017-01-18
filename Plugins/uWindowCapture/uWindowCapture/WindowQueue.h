#pragma once

#include <deque>
#include <mutex>


class WindowQueue
{
public:
    void Enqueue(int id);
    int Dequeue();
    bool Empty() const;

private:
    std::mutex mutex_;
    std::deque<int> queue_;
};
