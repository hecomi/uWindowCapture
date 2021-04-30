#include <algorithm>
#include "CaptureManager.h"
#include "WindowManager.h"
#include "Window.h"
#include "Debug.h"
#include "Util.h"

using namespace Microsoft::WRL;



namespace
{
    constexpr auto kLoopMinTime = std::chrono::microseconds(100);
}


// ---


CaptureManager::CaptureManager()
{
    windowCaptureThreadLoop_.SetFinalizer([]
    {
        if (auto& wgcManager = WindowManager::GetWindowsGraphicsCaptureManager())
        {
            wgcManager->StopAllInstances();
        }
    });

    windowCaptureThreadLoop_.Start([this] 
    {
        // at first, check the high-priority queue.
        int id = highPriorityQueue_.Dequeue();

        // move an item in the mid-priority queue to the high-priority queue to give a chance to it.
        if (id >= 0 && !middlePriorityQueue_.Empty())
        {
            const auto midId = middlePriorityQueue_.Dequeue();
            highPriorityQueue_.Enqueue(midId);
        }

        // second, check the mid-priority queue.
        if (id < 0)
        {
            id = middlePriorityQueue_.Dequeue();
        }

        // at last, check the low-priority queue.
        if (id < 0)
        {
            id = lowPriorityQueue_.Dequeue();
        }

        // update the window if needed.
        if (id >= 0 && WindowManager::Get().CheckExistence(id))
        {
            if (auto window = WindowManager::Get().GetWindow(id))
            {
                window->Capture();
            }
        }

        if (auto& wgcManager = WindowManager::GetWindowsGraphicsCaptureManager())
        {
            wgcManager->StopNonUpdatedInstances();
        }
    }, kLoopMinTime);

    iconCaptureThreadLoop_.Start([this] 
    {
        int id = iconQueue_.Dequeue();
        if (id >= 0 && WindowManager::Get().CheckExistence(id))
        {
            if (auto window = WindowManager::Get().GetWindow(id))
            {
                window->CaptureIcon();
            }
        }
    }, kLoopMinTime);
}


CaptureManager::~CaptureManager()
{
    iconCaptureThreadLoop_.Stop();
    windowCaptureThreadLoop_.Stop();
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


void CaptureManager::RequestCaptureIcon(int id)
{
    iconQueue_.Enqueue(id);
}