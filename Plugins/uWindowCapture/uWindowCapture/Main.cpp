#include <d3d11.h>
#include <dxgi1_2.h>
#include <Windows.h>
#include <wrl/client.h>
#include <memory>
#include <algorithm>
#include <vector>
#include <map>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"

#include "Common.h"
#include "Debug.h"
#include "Window.h"
#include "WindowManager.h"

#pragma comment(lib, "dxgi.lib")


// flag to check if this plugin has initialized.
bool g_hasInitialized = false;

// unity interafece to access ID3D11Device.
IUnityInterfaces* g_unity = nullptr;

// window manager instance.
std::unique_ptr<WindowManager> g_manager = nullptr;


bool CheckManager()
{
	if (!g_manager) 
	{
		Debug::Error("Manager has not been initialized.");
		return false;
	}
	return true;
}

std::shared_ptr<Window> GetWindow(int id)
{
	if (!CheckManager()) return nullptr;
	return g_manager->GetWindow(id);
}


extern "C"
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Initialize()
    {
		if (g_hasInitialized) return;
		g_hasInitialized = true;

        Debug::Initialize();
		g_manager = std::make_unique<WindowManager>();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API Finalize()
    {
		if (!g_hasInitialized) return;
		g_hasInitialized = false;

        Debug::Finalize();
		g_manager.reset();
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

    void UNITY_INTERFACE_API OnRenderEvent(int id)
    {
		if (auto window = GetWindow(id))
		{
			window->Capture();
			window->Draw();
		}
    }

    UNITY_INTERFACE_EXPORT UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UpdateWindowList()
    {
		if (!CheckManager()) return;
		g_manager->UpdateList();
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API GetWindowCount()
    {
		if (!CheckManager()) return -1;
		return static_cast<int>(g_manager->GetWindowList().size());
    }

    UNITY_INTERFACE_EXPORT const WindowInfo* UNITY_INTERFACE_API GetWindowList()
    {
		if (!CheckManager()) return nullptr;
		const auto& list = g_manager->GetWindowList();
		return &list[0];
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API AddWindow(HWND hwnd)
    {
		if (!CheckManager()) return -1;
		return g_manager->Add(hwnd);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API RemoveWindow(int id)
    {
		if (!CheckManager()) return;
		g_manager->Remove(id);
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API GetWidth(int id)
    {
		if (auto window = GetWindow(id))
		{
			return window->GetWidth();
		}
		return 0;
    }

    UNITY_INTERFACE_EXPORT UINT UNITY_INTERFACE_API GetHeight(int id)
    {
		if (auto window = GetWindow(id))
		{
			return window->GetHeight();
		}
		return 0;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API GetTitle(int id, wchar_t* buf, int len)
    {
        if (auto window = GetWindow(id))
        {
            window->GetTitle(buf, len);
        }
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API SetTexturePtr(int id, ID3D11Texture2D* ptr)
    {
		if (auto window = GetWindow(id))
		{
			return window->SetTexturePtr(ptr);
		}
    }
}