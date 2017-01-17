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
