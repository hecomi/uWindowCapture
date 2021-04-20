#pragma once

#include <functional>
#include <chrono>
#include <mutex>
#include <vector>
#include <list>
#include <dxgi.h>
#include <d3d11.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Capture.h>


class WindowsGraphicsCapture
    : public std::enable_shared_from_this<WindowsGraphicsCapture>
{
friend class WindowsGraphicsCaptureManager;
public:
    static bool IsAvailable();
    static bool IsCursorCaptureEnabledApiAvailable();

public:
    struct Result
    {
        ID3D11Texture2D *pTexture = nullptr;
        int width = 0;
        int height = 0;
        bool hasSizeChanged = false;
    };

    using Callback = std::function<void(const Result &)>;

    explicit WindowsGraphicsCapture(HWND hWnd);
    explicit WindowsGraphicsCapture(HMONITOR hMonitor);
    ~WindowsGraphicsCapture();
    int GetHeight() const { return size_.Height; }
    int GetWidth() const { return size_.Width; }
    void SetCallback(const Callback& callback) { callback_ = callback; }
    void Start();
    void RequestStop();
    void Stop();
    bool IsStarted() const { return isStarted_; }
    void EnableCursorCapture(bool enabled);
    Result TryGetLatestResult();
    void ChangePoolSize(int width, int height);

private:
    void Initialize();

    const HWND hWnd_;
    const HMONITOR hMonitor_;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice deviceWinRt_ = nullptr;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem graphicsCaptureItem_ = nullptr;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool captureFramePool_ = nullptr;
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession graphicsCaptureSession_ = nullptr;
    winrt::Windows::Graphics::SizeInt32 size_ = { 0, 0 };
    Callback callback_;
    bool isStarted_ = false;

    float stopTimer_ = 0.f;
    bool hasStopRequested_ = false;
};


class WindowsGraphicsCaptureManager
{
public:
    void Add(const std::shared_ptr<WindowsGraphicsCapture>& instance);
    void Remove(const std::shared_ptr<WindowsGraphicsCapture>& instance);
    void Update(float dt);
    void StopNonUpdatedInstances();
    void StopAllInstances();

private:
    using Ptr = std::shared_ptr<WindowsGraphicsCapture>;
    std::list<Ptr> instances_;
    std::vector<Ptr> removedInstances_;
    std::mutex instancesMutex_;
    std::mutex removedInstancesMutex_;
};