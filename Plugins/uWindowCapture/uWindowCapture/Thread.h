#pragma once

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>


// Interval
class Thread
{
public:
    using ThreadFunc = std::function<void()>;

    Thread();
    ~Thread();
    void Start(const ThreadFunc& func, const std::chrono::microseconds& interval);
    void Restart();
    void Stop();
    bool IsRunning() const;

private:
    std::thread thread_;
    std::atomic<bool> isRunning_ = false;
    std::chrono::microseconds interval_ = std::chrono::microseconds::zero();
    ThreadFunc func_ = nullptr;
};