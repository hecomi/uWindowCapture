#include <algorithm>
#include "CaptureManager.h"
#include "WindowManager.h"
#include "Window.h"
#include "Debug.h"
#include "Util.h"

using namespace Microsoft::WRL;



CaptureManager::CaptureManager()
{
    threadLoop_.Start([this] 
    {
        // at first, check high queue.
        int id = highPriorityQueue_.Dequeue();

        // move middle queue item to high queue to give chance to middle priority one.
        if (id >= 0 && !middlePriorityQueue_.Empty())
        {
            const auto midId = middlePriorityQueue_.Dequeue();
            highPriorityQueue_.Enqueue(midId);
        }

        // second, check imddle queue.
        if (id < 0)
        {
            id = middlePriorityQueue_.Dequeue();
        }

        // at last, check imddle queue.
        if (id < 0)
        {
            id = lowPriorityQueue_.Dequeue();
        }

        // update if needed.
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
    switch (priority)
    {
        case CapturePriority::High:
        {
            highPriorityQueue_.Enqueue(id);
            break;
        }
        case CapturePriority::Middle:
        {
            middlePriorityQueue_.Enqueue(id);
            break;
        }
        case CapturePriority::Low:
        {
            lowPriorityQueue_.Enqueue(id);
            break;
        }
    }
}
