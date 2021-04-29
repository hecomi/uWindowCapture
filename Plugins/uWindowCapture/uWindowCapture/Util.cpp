#pragma once

#include <vector>
#include "Util.h"
#include "Debug.h"



bool IsFullScreenWindow(HWND hWnd)
{
    const auto hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    if (!::GetMonitorInfo(hMonitor, &monitorInfo)) return false;
    const auto &monitor = monitorInfo.rcMonitor;
    const auto monitorWidth = monitor.right - monitor.left;
    const auto monitorHeight = monitor.bottom - monitor.top;

    RECT client;
    if (!::GetClientRect(hWnd, &client)) return false;

    return
        client.left == 0 &&
        client.top == 0 &&
        client.right == monitorWidth &&
        client.bottom == monitorHeight;
}


bool IsAltTabWindow(HWND hWnd)
{
    if (!::IsWindowVisible(hWnd)) return false;

    // Ref: https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863/
    HWND hWndWalk = ::GetAncestor(hWnd, GA_ROOTOWNER);
    HWND hWndTry;
    while ((hWndTry = ::GetLastActivePopup(hWndWalk)) != hWndTry) 
    {
        if (::IsWindowVisible(hWndTry)) break;
        hWndWalk = hWndTry;
    }
    if (hWndWalk != hWnd)
    {
        return false;
    }

    // Exclude tool windows
    if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
    {
        return false;
    }

    // Include fullscreen
    if (IsFullScreenWindow(hWnd))
    {
        return true;
    }

    // Exclude task tray programs
    TITLEBARINFO titleBar;
    titleBar.cbSize = sizeof(TITLEBARINFO);
    ::GetTitleBarInfo(hWnd, &titleBar);
    if (titleBar.rgstate[0] & STATE_SYSTEM_INVISIBLE)
    {
        return false;
    }

    return true;
}


bool IsUWP(DWORD pid)
{
    using GetPackageFamilyNameType = LONG (WINAPI*)(HANDLE, UINT32*, PWSTR);

    const auto hKernel32 = ::GetModuleHandleA("kernel32.dll");
    if (!hKernel32) return false;

    const auto GetPackageFamilyName = (GetPackageFamilyNameType)::GetProcAddress(hKernel32, "GetPackageFamilyName");
    if (!GetPackageFamilyName) return false;

    auto process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    ScopedReleaser releaser([&] { ::CloseHandle(process); });

    UINT32 len = 0;
    const auto res = GetPackageFamilyName(process, &len, NULL);

    return res == ERROR_INSUFFICIENT_BUFFER;
}


bool IsApplicationFrameWindow(const std::string& className)
{
    return className == "ApplicationFrameWindow";
}


UINT GetWindowZOrder(HWND hWnd)
{
    int z = 0;
    while (hWnd != NULL)
    {
        hWnd = ::GetWindow(hWnd, GW_HWNDPREV);
        if (::IsWindowVisible(hWnd) && ::IsWindow(hWnd) /* && ::GetWindowTextLength(hWnd) > 0*/)
        {
            ++z;
        }
    }
    return z;
}


bool GetWindowTitle(HWND hWnd, std::wstring& outTitle)
{
    const auto length = ::GetWindowTextLengthW(hWnd);
    if (length == 0) return false;

    std::vector<WCHAR> buf(length + 1);
    if (::GetWindowTextW(hWnd, &buf[0], static_cast<int>(buf.size())))
    {
         outTitle = &buf[0];
         return true;
    }

    return false;
}


bool GetWindowTitle(HWND hWnd, std::wstring& outTitle, int timeout)
{
    DWORD_PTR length = 0;
    LRESULT lr;

    lr = ::SendMessageTimeoutW(
        hWnd, 
        WM_GETTEXTLENGTH, 
        0, 
        0, 
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 
        timeout, 
        reinterpret_cast<PDWORD_PTR>(&length));
    if (FAILED(lr)) return false;

    if (length > 256) return false;

    std::vector<WCHAR> buf(length + 1);
    DWORD_PTR result;

    lr = ::SendMessageTimeoutW(
        hWnd, 
        WM_GETTEXT, 
        buf.size(), 
        reinterpret_cast<LPARAM>(&buf[0]), 
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 
        timeout, 
        reinterpret_cast<PDWORD_PTR>(&result));
    if (FAILED(lr)) return false;

    outTitle = &buf[0];

    return true;
}


bool GetWindowClassName(HWND hWnd, std::string& outClassName)
{
    constexpr size_t maxLength = 128;

    char buf[maxLength];
    if (::GetClassName(hWnd, buf, maxLength))
    {
        outClassName = buf;
        return true;
    }

    return false;
}


ScopedTimer::ScopedTimer(TimerFuncType&& func)
    : func_(func)
    , start_(std::chrono::high_resolution_clock::now())
{
}


ScopedTimer::~ScopedTimer()
{
    func_(GetElapsedTime());
}


ScopedTimer::microseconds ScopedTimer::GetElapsedTime() const
{
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<microseconds>(end - start_);
}
