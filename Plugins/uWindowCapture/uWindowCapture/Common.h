#pragma once

#include <memory>


// Unity interface and ID3D11Device getters
struct IUnityInterfaces* GetUnity();

struct ID3D11Device* GetUnityDevice();

// Window manager getter
std::unique_ptr<class WindowManager>& GetWindowManager();
