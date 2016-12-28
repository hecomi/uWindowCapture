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