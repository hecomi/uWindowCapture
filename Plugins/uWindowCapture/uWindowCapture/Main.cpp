#include <d3d11.h>
#include <dxgi1_2.h>
#include <Windows.h>
#include <wrl/client.h>
#include <memory>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

#include "Common.h"
#include "Debug.h"

#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

IUnityInterfaces* g_unity = nullptr;
ID3D11Texture2D* g_ptr = nullptr;

ComPtr<IDXGISurface1> g_surface;
ComPtr<ID3D11Texture2D> g_texture;
int g_width = -1, g_height = -1;

Buffer<BYTE> g_buffer;


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Initialize()
    {
        Debug::Initialize();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Finalize()
    {
        Debug::Finalize();
    }

    void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType event)
    {
        switch (event)
        {
            case kUnityGfxDeviceEventInitialize:
            {
                Initialize();
                break;
            }
            case kUnityGfxDeviceEventShutdown:
            {
                Finalize();
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

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
        auto hwnd = GetForegroundWindow();

        if (!IsWindow(hwnd))
        {
            Debug::Error("hwnd is not a window handle.");
            return;
        }
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetDebugMode(Debug::Mode mode)
    {
        Debug::SetMode(mode);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetLogFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetLogFunc(func);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetErrorFunc(Debug::DebugLogFuncPtr func)
    {
        Debug::SetErrorFunc(func);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetWindowTexture(ID3D11Texture2D* ptr)
    {
        g_ptr = ptr;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWidth()
    {
        auto hwnd = GetForegroundWindow();
        RECT rect;
        GetWindowRect(hwnd, &rect);
        return rect.right - rect.left;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetHeight()
    {
        auto hwnd = GetForegroundWindow();
        RECT rect;
        GetWindowRect(hwnd, &rect);
        return rect.bottom - rect.top;
    }
}