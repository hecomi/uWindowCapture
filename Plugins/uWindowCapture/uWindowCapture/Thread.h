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
    using microseconds = std::chrono::microseconds;

    Thread();
    ~Thread();
    void Start(
        const ThreadFunc& func,
        const microseconds& interval = microseconds(1'000'000 / 60));
    void Restart();
    void Stop();
    bool IsRunning() const;

private:
    std::thread thread_;
    std::atomic<bool> isRunning_ = false;
    microseconds interval_ = microseconds::zero();
    ThreadFunc func_ = nullptr;
};