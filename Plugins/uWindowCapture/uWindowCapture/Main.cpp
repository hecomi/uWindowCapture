#include <d3d11.h>
#include <dxgi1_2.h>
#include <Windows.h>
#include <memory>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

#include "Debug.h"
#include "Message.h"
#include "Uploader.h"
#include "Window.h"
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

        Uploader::Create();
        Uploader::Get().Initialize();

        WindowManager::Create();
        WindowManager::Get().Initialize();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcFinalize()
    {
        if (!g_hasInitialized) return;
        g_hasInitialized = false;

        WindowManager::Get().Finalize();
        WindowManager::Destroy();

        Uploader::Get().Finalize();
        Uploader::Destroy();

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

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcStartCaptureWindow(int id)
    {
        if (auto window = GetWindow(id))
        {
            window->StartCapture();
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcStopCaptureWindow(int id)
    {
        if (auto window = GetWindow(id))
        {
            window->StopCapture();
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcRequestCaptureWindow(int id)
    {
        if (auto window = GetWindow(id))
        {
            window->RequestCapture();
        }
    }

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetWindowOwner(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetOwner();
        }
        return nullptr;
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

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcUpdateWindowTitle(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->UpdateTitle();
        }
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
            return window->GetTexturePtr();
        }
        return nullptr;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetWindowTexturePtr(int id, ID3D11Texture2D* ptr)
    {
        if (auto window = GetWindow(id))
        {
            window->SetTexturePtr(ptr);
        }
    }

    UNITY_INTERFACE_EXPORT Window::CaptureMode UNITY_INTERFACE_API UwcGetWindowCaptureMode(int id)
    {
        if (auto window = GetWindow(id))
        {
            return window->GetCaptureMode();
        }
        return Window::CaptureMode::None;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UwcSetWindowCaptureMode(int id, Window::CaptureMode mode)
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

    UNITY_INTERFACE_EXPORT HWND UNITY_INTERFACE_API UwcGetForegroundWindow()
    {
        return GetForegroundWindow();
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetScreenWidth()
    {
        return GetSystemMetrics(SM_CXVIRTUALSCREEN);
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API UwcGetScreenHeight()
    {
        return GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
}