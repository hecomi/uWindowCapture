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

    using Callback = std::function<void(const Result &)>;

    explicit WindowsGraphicsCapture(HWND hWnd);
    explicit WindowsGraphicsCapture(HMONITOR hMonitor);
    ~WindowsGraphicsCapture();
    void Update(float dt);
    int GetHeight() const { return size_.Height; }
    int GetWidth() const { return size_.Width; }
    void SetCallback(const Callback& callback) { callback_ = callback; }
    void Start();
    void RequestStop();
    void Stop(bool removeFromManager = true);
    bool ShouldStop() const;
    bool IsSessionRestartRequested() const { return isSessionRestartRequested_; }
    void Restart();
    bool IsAvailable() const;
    bool IsStarted() const { return isStarted_; }
    bool IsValid() const { return static_cast<bool>(session_); }
    void EnableCursorCapture(bool enabled);
    Result TryGetLatestResult();
    void ReleaseLatestResult();
    void ReleaseFrame();
    void ChangePoolSize(int width, int height);
    const wchar_t * GetDisplayName() const;

private:
    void CreateItem();
    bool CreatePoolAndSession();
    void DestroyPoolAndSession();

    const HWND hWnd_;
    const HMONITOR hMonitor_;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item_ = nullptr;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool pool_ = nullptr;
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session_ = nullptr;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame_ = nullptr;
    winrt::Windows::Graphics::SizeInt32 size_ = { 0, 0 };
    Callback callback_;
    bool isStarted_ = false;
    std::mutex sessionAndPoolMutex_;
    std::chrono::time_point<std::chrono::steady_clock> latestFrameTime_;

    std::atomic<float> stopTimer_ = { 0.f };
    std::atomic<bool> hasStopRequested_ = { false };
    std::atomic<bool> isCursorCaptureEnabled_ = { true };
    std::atomic<bool> isSessionRestartRequested_ = { false };
};


class WindowsGraphicsCaptureManager final
{
public:
    void Add(const std::shared_ptr<WindowsGraphicsCapture>& instance);
    void Remove(const std::shared_ptr<WindowsGraphicsCapture>& instance);
    void UpdateFromMainThread(float dt);
    void UpdateFromCaptureThread();
    void StopAllInstances();
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice & GetDevice();

private:
    void UpdateAddInstances();
    void UpdateRemoveInstances();

    using Ptr = std::shared_ptr<WindowsGraphicsCapture>;
    std::list<Ptr> instances_;
    std::vector<Ptr> addedInstances_;
    std::vector<Ptr> removedInstances_;
    std::mutex instancesMutex_;
    std::mutex addedInstancesMutex_;
    std::mutex removedInstancesMutex_;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice deviceWinRt_ = nullptr;
};