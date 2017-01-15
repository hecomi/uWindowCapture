#include <algorithm>
#include "CaptureManager.h"
#include "WindowManager.h"
#include "Window.h"
#include "Debug.h"
#include "Util.h"

using namespace Microsoft::WRL;



void CaptureQueue::Enqueue(int id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = std::find(queue_.begin(), queue_.end(), id);
    if (it == queue_.end())
    {
        queue_.push_front(id);
    }
}


int CaptureQueue::Dequeue()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty()) return -1;

    const auto id = queue_.back();
    queue_.pop_back();
    return id;
}



// ---



CaptureManager::CaptureManager()
{
    threadLoop_.Start([this] 
    {
        UWC_SCOPE_TIMER(WindowCapture)

        int id = highPriorityQueue_.Dequeue();
        if (id < 0)
        {
            id = lowPriorityQueue_.Dequeue();
        }

        if (id >= 0)
        {
            if (auto window = WindowManager::Get().GetWindow(id))
            {
                window->Capture();
            }
        }
    }, std::chrono::microseconds(100));
}


CaptureManager::~CaptureManager()
{
    threadLoop_.Stop();
}


void CaptureManager::RequestCapture(int id, CapturePriority priority)
{
    auto window = WindowManager::Get().GetWindow(id);
    if (!window) return;

    switch (priority)
    {
        case CapturePriority::High:
        {
            highPriorityQueue_.Enqueue(id);
            break;
        }
        case CapturePriority::Low:
        {
            lowPriorityQueue_.Enqueue(id);
            break;
        }
    }
}
