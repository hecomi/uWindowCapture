#pragma once

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>


// Interval
class Thread
{
public:
    Thread();
    ~Thread();
    void Start();
    void Stop();

private:
    std::atomic<bool> isRunning_;
};