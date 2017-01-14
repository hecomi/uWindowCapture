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


Thread::Thread()
{
}


Thread::~Thread()
{
    Stop();
}


void Thread::Start(const ThreadFunc& func, const std::chrono::microseconds& interval)
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
        for (;;)
        {
            ScopedThreadSleeper sleeper(interval_);
            if (isRunning_)
            {
                func_();
            }
            else
            {
                break;
            }
        }
    });
}


void Thread::Restart()
{
    Start(func_, interval_);
}


void Thread::Stop()
{
    if (!isRunning_) return;

    isRunning_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}


bool Thread::IsRunning() const
{
    return isRunning_;
}