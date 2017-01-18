#pragma once

#include <Windows.h>
#include <deque>
#include <mutex>

#include "WindowQueue.h"
#include "Thread.h"


enum class CapturePriority
{
    High = 0,
    Middle = 1,
    Low  = 2,
};


class CaptureManager
{
public:
    CaptureManager();
    ~CaptureManager();
    void RequestCapture(int id, CapturePriority priority);

private:
    ThreadLoop threadLoop_;
    WindowQueue highPriorityQueue_;
    WindowQueue middlePriorityQueue_;
    WindowQueue lowPriorityQueue_;
};