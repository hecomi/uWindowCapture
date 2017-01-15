#pragma once

#include <Windows.h>
#include <deque>
#include <mutex>

#include "Thread.h"


enum class CapturePriority
{
    High = 0,
    Low  = 1,
};


class CaptureQueue
{
public:
    void Enqueue(int id);
    int Dequeue();

private:
    std::mutex mutex_;
    std::deque<int> queue_;
};


class CaptureManager
{
public:
    CaptureManager();
    ~CaptureManager();
    void RequestCapture(int id, CapturePriority priority);

private:
    ThreadLoop threadLoop_;
    CaptureQueue highPriorityQueue_;
    CaptureQueue lowPriorityQueue_;
};