#pragma once

#include "Thread.h"
#include "Debug.h"
#include "Util.h"



class ScopedThreadSleeper : public ScopedTimer
{
public:
    template <class T>
    explicit ScopedThreadSleeper(const T& duration) : 
        ScopedTimer([duration] (microseconds us)
        {
            const auto waitTime = duration - us;
            if (waitTime > microseconds::zero())
            {
                std::this_thread::sleep_for(waitTime);
            }
        }) 
    {}
};


ThreadLoop::ThreadLoop()
{
}


ThreadLoop::~ThreadLoop()
{
    Stop();
}


void ThreadLoop::Start(const ThreadFunc& func, const microseconds& interval)
{
    if (isRunning_) return;

    func_ = func;
    interval_ = interval;
    isRunning_ = true;

    if (thread_.joinable())
    {
        Debug::Error(__FUNCTION__, " => Thread is running");
        thread_.join();
    }

    thread_ = std::thread([this] 
    {
        while (isRunning_)
        {
            ScopedThreadSleeper sleeper(std::chrono::microseconds(1000000 / 60));// interval_);
            func_();
        }
    });
}


void ThreadLoop::Restart()
{
    Start(func_, interval_);
}


void ThreadLoop::Stop()
{
    if (!isRunning_) return;

    isRunning_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}


bool ThreadLoop::IsRunning() const
{
    return isRunning_;
}