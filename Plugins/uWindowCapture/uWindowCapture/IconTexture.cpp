#include <fstream>
#include <regex>
#include <string>
#include <shlwapi.h>
#include <gdiplus.h>
#include "IconTexture.h"
#include "Window.h"
#include "WindowManager.h"
#include "UploadManager.h"
#include "Debug.h"
#include "Unity.h"
#include "Util.h"
#include "Message.h"

#pragma comment(lib, "shlwapi")
#pragma	comment(lib, "gdiplus")

using namespace Microsoft::WRL;



IconTexture::IconTexture(Window* window)
    : window_(window)
{
}


IconTexture::~IconTexture()
{
    if (!appLogoPath_.empty())
    {
        ::DestroyIcon(hIcon_);
    }
}


void IconTexture::InitIcon()
{
    try
    {
        if (window_->IsUWP())
        {
            InitIconHandleForStoreApp();
        }
        else
        {
            InitIconHandleForWin32App();
        }
    }
    catch (const std::exception &e)
    {
        Debug::Error(__FUNCTION__, " => Exception ", e.what());
    }

    if (!hIcon_)
    {
        hIcon_ = ::LoadIcon(0, IDI_APPLICATION);
    }

    if (!hIcon_) return;

    ICONINFO info;
    if (!::GetIconInfo(hIcon_, &info))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return;
    }
    ScopedReleaser iconReleaser([&] 
    { 
        ::DeleteObject(info.hbmColor); 
        ::DeleteObject(info.hbmMask); 
    });

    BITMAP bmpColor { 0 };
    ::GetObject(info.hbmColor, sizeof(bmpColor), &bmpColor);
    width_ = bmpColor.bmWidth;
    height_ = bmpColor.bmHeight;
}


void IconTexture::InitIconHandleForWin32App()
{
    // Now we cannot get an icon from UWP app from these methods.

    const auto hWnd = window_->GetWindowHandle();

    hIcon_ = reinterpret_cast<HICON>(::GetClassLongPtr(hWnd, GCLP_HICON));
    if (hIcon_) return;

    constexpr UINT timeout = 100;
    auto lr = ::SendMessageTimeoutW(
        hWnd, 
        WM_GETICON, 
        ICON_BIG, 
        0, 
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 
        timeout, 
        reinterpret_cast<PDWORD_PTR>(&hIcon_));
    if (hIcon_ && SUCCEEDED(lr)) return;

    Debug::Error(__FUNCTION__, " => Could not get HICON.");
    return;
}


void IconTexture::InitIconHandleForStoreApp()
{
    DWORD processId = 0;
    if (window_->IsApplicationFrameWindow())
    {
        processId = GetStoreAppProcessId(window_->GetWindowHandle());
        if (processId == 0)
        {
            processId = window_->GetProcessId();
        }
    }
    else
    {
        processId = window_->GetProcessId();
    }

    const auto hProcess = ::OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        processId);
    if (!hProcess) return;
    ScopedReleaser processCloser([&] { ::CloseHandle(hProcess); });

    WCHAR dirPathBuf[512];
    try
    {
        DWORD len = 512;
        if (!::QueryFullProcessImageNameW(hProcess, 0, dirPathBuf, &len))
        {
            OutputApiError(__FUNCTION__, "QueryFullProcessImageNameW");
            return;
        }
    }
    catch (...)
    {
        OutputApiError(__FUNCTION__, "QueryFullProcessImageNameW");
        return;
    }

    if (!::PathRemoveFileSpecW(dirPathBuf)) return;
    const std::wstring dirPath = dirPathBuf;

    const auto appManifestPath = dirPath + L"\\AppxManifest.xml";
    std::wifstream fs(appManifestPath);
    if (!fs) return;

    std::wstring logoRelativePath;
    {
        std::wregex regex(L"<Logo>([^<]+)</Logo>");
        std::wstring line;
        while (std::getline(fs, line))
        {
            std::wsmatch match {};
            if (std::regex_search(line, match, regex))
            {
                logoRelativePath = match[1].str().c_str();
                break;
            }
        }
        if (logoRelativePath.empty()) return;
    }

    const auto logoPath = std::wstring(dirPath) + L"\\" + logoRelativePath;
    if (::PathFileExistsW(logoPath.c_str()))
    {
        appLogoPath_ = logoPath;
        CreateIconFromAppLogoPath();
        return;
    }

    std::wstring logoDirRelativePath;
    std::wstring logoFileName;
    std::wstring logoFileExt;
    {
        std::wregex regex(L"(.+)\\\\([^\\.\\\\]+?)\\.(.+)$");
        std::wsmatch match {};
        if (!std::regex_search(logoRelativePath, match, regex)) return;

        logoDirRelativePath = match[1].str();
        logoFileName = match[2].str();
        logoFileExt = match[3].str();
    }

    const auto searchQuery = 
        dirPath + L"\\" + 
        logoDirRelativePath + L"\\" + 
        logoFileName + L".scale-*." + logoFileExt;

    WIN32_FIND_DATAW fd;
    auto hFile = ::FindFirstFileW(searchQuery.c_str(), &fd);
    ScopedReleaser fileCloser([&] { ::FindClose(hFile); });

    int maxScale = 0;
    do
    {
        const std::wstring fileName = fd.cFileName;
        const auto query = L"\\.scale-([^\\.]+)\\.";
        std::wregex regex(query);
        std::wsmatch match {};
        if (std::regex_search(fileName, match, regex))
        {
            const int scale = std::stoi(match[1].str());
            if (scale > maxScale)
            {
                maxScale = scale;
            }
        }
    } 
    while (FindNextFileW(hFile, &fd));

    if (maxScale == 0) return;

    appLogoPath_ = 
        dirPath + L"\\" + 
        logoDirRelativePath + L"\\" +
        logoFileName + L".scale-" + 
        std::to_wstring(maxScale) +
        L"." + logoFileExt;
    CreateIconFromAppLogoPath();
}


void IconTexture::CreateIconFromAppLogoPath()
{
    if (appLogoPath_.empty()) return;

    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &input, nullptr);
    ScopedReleaser gdiShutdowner([&] { Gdiplus::GdiplusShutdown(token); });

    auto image = Gdiplus::Bitmap::FromFile(appLogoPath_.c_str());
    if (!image || image->GetLastStatus() != Gdiplus::Ok) return;
    ScopedReleaser imageDeleter([&] { delete image; });

    width_ = image->GetWidth();
    height_ = image->GetHeight();
    image->GetHICON(&hIcon_);
}


UINT IconTexture::GetWidth() const
{
    return width_ > 0 ? width_ : ::GetSystemMetrics(SM_CXICON);
}


UINT IconTexture::GetHeight() const
{
    return height_ > 0 ? height_ : ::GetSystemMetrics(SM_CYICON);
}


void IconTexture::SetUnityTexturePtr(ID3D11Texture2D* ptr)
{
    unityTexture_ = ptr;
}


ID3D11Texture2D* IconTexture::GetUnityTexturePtr() const
{
    return unityTexture_;
}


bool IconTexture::Capture()
{
    if (!hIcon_) return false;

    ICONINFO info;
    if (!::GetIconInfo(hIcon_, &info))
    {
        OutputApiError(__FUNCTION__, "GetIconInfo");
        return false;
    }
    ScopedReleaser iconReleaser([&] 
    { 
        ::DeleteObject(info.hbmColor); 
        ::DeleteObject(info.hbmMask); 
    });

    auto hDcMem = ::CreateCompatibleDC(NULL);
    ScopedReleaser hDcReleaser([&] { ::DeleteDC(hDcMem); });

    BITMAPINFOHEADER bmi {};
    bmi.biWidth       = static_cast<LONG>(width_);
    bmi.biHeight      = -static_cast<LONG>(height_);
    bmi.biPlanes      = 1;
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biBitCount    = 32;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage   = 0;

    Buffer<BYTE> color;
    color.ExpandIfNeeded(width_ * height_ * 4);
    if (!::GetDIBits(hDcMem, info.hbmColor, 0, height_, color.Get(), reinterpret_cast<BITMAPINFO*>(&bmi), DIB_RGB_COLORS))
    {
        OutputApiError(__FUNCTION__, "GetDIBits");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        buffer_.ExpandIfNeeded(width_ * height_ * 4);

        auto* buffer32 = buffer_.As<UINT>();
        const auto* color32 = color.As<UINT>();
        bool areAllPixelsAlphaZero = true;

        const auto n = width_ * height_;
        for (UINT x = 0; x < width_; ++x) 
        {
            for (UINT y = 0; y < height_; ++y)
            {
                const auto i = y * width_ + x;
                const auto j = (height_ - 1 - y) * width_ + x;
                buffer32[j] = color32[i];
                if ((color32[i] & 0xff000000) != 0) areAllPixelsAlphaZero = false;
            }
        }

        if (areAllPixelsAlphaZero)
        {
            for (UINT i = 0; i < n; ++i)
            {
                buffer32[i] |= 0xff000000;
            }
        }
    }

    hasCaptured_ = true;

    return true;
}


bool IconTexture::CaptureOnce()
{
    if (hasCaptured_) return false;

    return Capture();
}


bool IconTexture::Upload()
{
    if (!unityTexture_.load() || buffer_.Empty()) return false;

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    {
        D3D11_TEXTURE2D_DESC desc;
        unityTexture_.load()->GetDesc(&desc);
        if (desc.Width != GetWidth() || desc.Height != GetHeight())
        {
            Debug::Error(__FUNCTION__, " => Texture size is wrong.");
            return false;
        }
    }

    auto& uploader = WindowManager::GetUploadManager();
    if (!uploader) return false;

    sharedTexture_ = uploader->CreateCompatibleSharedTexture(unityTexture_.load());
    if (!sharedTexture_)
    {
        Debug::Error(__FUNCTION__, " => Shared texture is null.");
        return false;
    }

    ComPtr<IDXGIResource> dxgiResource;
    sharedTexture_.As(&dxgiResource);
    if (FAILED(dxgiResource->GetSharedHandle(&sharedHandle_)))
    {
        Debug::Error(__FUNCTION__, " => GetSharedHandle() failed.");
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        ComPtr<ID3D11DeviceContext> context;
        uploader->GetDevice()->GetImmediateContext(&context);
        context->UpdateSubresource(sharedTexture_.Get(), 0, nullptr, buffer_.Get(), GetWidth() * 4, 0);
        context->Flush();
    }

    hasUploaded_ = true;

    return true;
}


bool IconTexture::UploadOnce()
{
    if (hasUploaded_) return false;

    return Upload();
}


bool IconTexture::Render()
{
    if (!unityTexture_.load() || !sharedTexture_ || !sharedHandle_) return false;

    std::lock_guard<std::mutex> lock(sharedTextureMutex_);

    ComPtr<ID3D11DeviceContext> context;
    GetUnityDevice()->GetImmediateContext(&context);

    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(GetUnityDevice()->OpenSharedResource(sharedHandle_, __uuidof(ID3D11Texture2D), &texture)))
    {
        Debug::Error(__FUNCTION__, " => OpenSharedResource() failed.");
        return false;
    }

    context->CopyResource(unityTexture_.load(), texture.Get());

    MessageManager::Get().Add({ MessageType::IconCaptured, window_->GetId(), window_->GetWindowHandle() });

    return true;
}


bool IconTexture::RenderOnce()
{
    if (hasRendered_) return true;

    return Render();
}