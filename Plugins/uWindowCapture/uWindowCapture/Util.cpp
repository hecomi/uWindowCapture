#pragma once

#include <vector>
#include "Util.h"



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


ScopedTimer::ScopedTimer(TimerFuncType&& func)
    : func_(func)
    , start_(std::chrono::high_resolution_clock::now())
{
}


ScopedTimer::~ScopedTimer()
{
    const auto end = std::chrono::high_resolution_clock::now();
    const auto time = std::chrono::duration_cast<microseconds>(end - start_);
    func_(time);
}
