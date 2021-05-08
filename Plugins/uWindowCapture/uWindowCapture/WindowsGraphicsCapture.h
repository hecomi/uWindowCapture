#pragma once

#include <functional>
#include <chrono>
#include <mutex>
#include <set>
#include <list>
#include <atomic>
#include <dxgi.h>
#include <d3d11.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Capture.h>


class WindowsGraphicsCapture
    : public std::enable_shared_from_this<WindowsGraphicsCapture>
{
friend class WindowsGraphicsCaptureManager;
public:
    static bool IsSupported();
    static bool IsCursorCaptureEnabledApiSupported();

public:
    struct Result
    {
        ID3D11Texture2D *pTexture = nullptr;
        int width = 0;
        int height = 0;
        bool hasSizeChanged = false;
    };

    explicit WindowsGraphicsCapture(HWND hWnd);
    explicit WindowsGraphicsCapture(HMONITOR hMonitor);
    ~WindowsGraphicsCapture();
    void Update(float dt);
    int GetHeight() const { return size_.Height; }
    int GetWidth() const { return size_.Width; }
    void RequestStart();
    bool IsAvailable() const;
    bool IsStarted() const { return isStarted_; }
    void EnableCursorCapture(bool enabled);
    Result TryGetLatestResult();
    void ReleaseLatestResult();
    void ReleaseFrame();
    void ChangePoolSize(int width, int height);
    const wchar_t * GetDisplayName() const;

private:
    bool ShouldStop() const;
    bool ShouldRestart() const;
    void Restart();
    void Start();
    void Stop();
    bool CreateItem();
    bool CreatePoolAndSession();
    void DestroyPoolAndSession();

    const HWND hWnd_;
    const HMONITOR hMonitor_;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item_ = nullptr;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool pool_ = nullptr;
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session_ = nullptr;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame_ = nullptr;
    winrt::Windows::Graphics::SizeInt32 size_ = { 0, 0 };
    mutable std::mutex itemMutex_;
    std::mutex sessionAndPoolMutex_;
    std::atomic<bool> isStarted_ = false;
    std::atomic<bool> isCursorCaptureEnabled_ = { true };
    std::atomic<bool> isStartRequested_ = { false };
    std::atomic<float> stopTimer_ = { 0.f };
    std::atomic<bool> isRestartRequested_ = { false };
    std::atomic<float> restartTimer_ = { 0.f };
};


class WindowsGraphicsCaptureManager final
{
public:
    std::shared_ptr<WindowsGraphicsCapture> Create(HWND hWnd);
    std::shared_ptr<WindowsGraphicsCapture> Create(HMONITOR hMonitor);
    void Destroy(const std::shared_ptr<WindowsGraphicsCapture>& instance);

    void UpdateFromMainThread(float dt);
    void UpdateFromCaptureThread();

    void StopAllInstances();
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice & GetDevice();

private:
    void StartInstances();
    void RestartInstances();
    void StopInstances();

    using Ptr = std::shared_ptr<WindowsGraphicsCapture>;
    std::list<Ptr> allInstances_;
    std::list<Ptr> activeInstances_;
    std::mutex allInstancesMutex_;
    std::mutex activeInstancesMutex_;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice deviceWinRt_ = nullptr;
};