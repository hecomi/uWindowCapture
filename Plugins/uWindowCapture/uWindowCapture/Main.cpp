#include <d3d11.h>
#include <dxgi1_2.h>
#include <Windows.h>
#include <memory>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

#include "Debug.h"
#include "Message.h"
#include "UploadManager.h"
#include "CaptureManager.h"
#include "Window.h"
#include "WindowTexture.h"
#include "WindowManager.h"

#pragma comment(lib, "dxgi.lib")


// flag to check if this plugin has initialized.
bool g_hasInitialized = false;

// unity interafece to access ID3D11Device.
IUnityInterfaces* g_unity = nullptr;


std::shared_ptr<Window> GetWindow(int id)
{
    if (WindowManager::IsNull()) return nullptr;
    return WindowManager::Get().GetWindow(id);
}


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcInitialize()
    {
        if (g_hasInitialized) return;
        g_hasInitialized = true;

        Debug::Initialize();

        MessageManager::Create();

        WindowManager::Create();
        WindowManager::Get().Initialize();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcFinalize()
    {
        if (!g_hasInitialized) return;
        g_hasInitialized = false;

        WindowManager::Get().Finalize();
        WindowManager::Destroy();

        MessageManager::Destroy();

        Debug::Finalize();
    }

    void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType event)
    {
        switch (event)
        {
        case kUnityGfxDeviceEventInitialize:
        {
            UwcInitialize();
            break;
        }
        case kUnityGfxDeviceEventShutdown:
        {
            UwcFinalize();
            break;
        }
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        g_unity = unityInterfaces;
        auto unityGraphics = g_unity->Get<IUnityGraphics>();
        unityGraphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        auto unityGraphics = g_unity->Get<IUnityGraphics>();
        unityGraphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
        g_unity = nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetDebugMode(Debug::Mode mode)
    {
        Debug::SetMode(mode);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetLogFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetLogFunc(func);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetErrorFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetErrorFunc(func);
    }

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        if (WindowManager::IsNull()) return;
        WindowManager::Get().Render();
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API UwcGetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcUpdate()
    {
        if (WindowManager::IsNull()) return;
        WindowManager::Get().Update();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcTriggerGpuUpload()
    {
        if (WindowManager::IsNull()) return;
        WindowManager::Get().GetUploadManager()->TriggerGpuUpload();
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetMessageCount()
    {
        if (MessageManager::IsNull()) return 0;
        return MessageManager::Get().GetCount();
    }

    UNITY_INTERFACE_EXPORT const Message* UNITY_INTERFACE_API UwcGetMessages()
    {
        if (MessageManager::IsNull()) return nullptr;
        return MessageManager::Get().GetHeadPointer();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcClearMessages()
    {
        if (MessageManager::IsNull()) return;
        MessageManager::Get().ClearAll();
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowHandle(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetHandle();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcRequestCaptureWindow(int id, CapturePriority priority)
    {
        if (WindowManager::IsNull()) return;
        WindowManager::GetCaptureManager()->RequestCapture(id, priority);
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowOwner(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetOwner();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowParent(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetParent();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT HINSTANCE UNITY_INTERFACE_API UwcGetWindowInstance(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetInstance();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT DWORD UNITY_INTERFACE_API UwcGetWindowProcessId(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetProcessId();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT DWORD UNITY_INTERFACE_API UwcGetWindowThreadId(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetThreadId();
        }
        return -1;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowX(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetX();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowY(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetY();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowWidth(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetWidth();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowHeight(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetHeight();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowZOrder(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetZOrder();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowBufferWidth(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetBufferWidth();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowBufferHeight(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetBufferHeight();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowIconWidth(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetIconWidth();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowIconHeight(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetIconHeight();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetWindowTitleLength(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetTitleLength();
        }
        return 0;
    }

    UNITY_INTERFACE_EXPORT const WCHAR* UNITY_INTERFACE_API UwcGetWindowTitle(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetTitle().c_str();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT ID3D11Texture2D* UNITY_INTERFACE_API UwcGetWindowTexturePtr(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetWindowTexture();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetWindowTexturePtr(int id, ID3D11Texture2D* ptr)
    {
        if (auto window = GetWindow(id))
        {
            window->SetWindowTexture(ptr);
        }
    }

    UNITY_INTERFACE_EXPORT ID3D11Texture2D* UNITY_INTERFACE_API UwcGetWindowIconTexturePtr(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetIconTexture();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetWindowIconTexturePtr(int id, ID3D11Texture2D* ptr)
    {
        if (auto window = GetWindow(id))
        {
            window->SetIconTexture(ptr);
        }
    }

    UNITY_INTERFACE_EXPORT CaptureMode UNITY_INTERFACE_API UwcGetWindowCaptureMode(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetCaptureMode();
        }
        return CaptureMode::None;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetWindowCaptureMode(int id, CaptureMode mode)
    {
        if (auto window = GetWindow(id))
        {
            return window->SetCaptureMode(mode);
        }
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindow(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsWindow() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsAltTabWindow(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsAltTab();
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsDesktop(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsDesktop();
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowVisible(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsVisible() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowEnabled(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsEnabled() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowUnicode(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsUnicode() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowZoomed(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsZoomed() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowIconic(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsIconic() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowHungUp(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsHungUp() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API UwcIsWindowTouchable(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->IsTouchable() > 0;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcMoveWindow(int id, int x, int y)
    {
        if (auto window = GetWindow(id))
        {
            window->MoveWindow(x, y);
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcScaleWindow(int id, int width, int height)
    {
        if (auto window = GetWindow(id))
        {
            window->ScaleWindow(width, height);
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcMoveAndScaleWindow(int id, int x, int y, int width, int height)
    {
        if (auto window = GetWindow(id))
        {
            window->MoveAndScaleWindow(x, y, width, height);
        }
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetForegroundWindow()
    {
        return ::GetForegroundWindow();
    }

    UNITY_INTERFACE_EXPORT POINT UNITY_INTERFACE_API UwcGetCursorPosition()
    {
        POINT point;
        GetCursorPos(&point);
        return point;
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowFromPoint(int x, int y)
    {
        if (WindowManager::IsNull()) return nullptr;
        if (auto window = WindowManager::Get().GetWindowFromPoint({ x, y }))
        {
            return window->GetHandle();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowUnderCursor()
    {
        if (WindowManager::IsNull()) return nullptr;
        if (auto window = WindowManager::Get().GetCursorWindow())
        {
            return window->GetHandle();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetScreenWidth()
    {
        return ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetScreenHeight()
    {
        return ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
}