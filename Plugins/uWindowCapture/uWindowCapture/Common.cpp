#pragma once

#include <d3d11.h>
#include "IUnityInterface.h"
#include "IUnityGraphicsD3D11.h"
#include "WindowManager.h"
#include "Common.h"


extern IUnityInterfaces* g_unity;
extern std::unique_ptr<WindowManager> g_manager;


IUnityInterfaces* GetUnity()
{
    return g_unity;
}


ID3D11Device* GetDevice()
{
    return GetUnity()->Get<IUnityGraphicsD3D11>()->GetDevice();
}


std::unique_ptr<WindowManager>& GetWindowManager()
{
    return g_manager;
}


bool IsAltTabWindow(HWND hWnd)
{
    if (!::IsWindowVisible(hWnd)) return false;

    // Ref: https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
    HWND hWndWalk = ::GetAncestor(hWnd, GA_ROOTOWNER);
    HWND hWndTry;
    while ((hWndTry = ::GetLastActivePopup(hWndWalk)) != hWndTry) {
        if (::IsWindowVisible(hWndTry)) break;
        hWndWalk = hWndTry;
    }
    if (hWndWalk != hWnd)
    {
        return false;
    }

    // Tool window
    if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
    {
        return false;
    }

    // Remove task tray programs
    TITLEBARINFO titleBar;
    titleBar.cbSize = sizeof(TITLEBARINFO);
    ::GetTitleBarInfo(hWnd, &titleBar);
    if (titleBar.rgstate[0] & STATE_SYSTEM_INVISIBLE)
    {
        return false;
    }

    return true;
}
