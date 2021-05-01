#pragma once

#include <functional>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>


class ThreadLoop
{
public:
    using ThreadFunc = std::function<void()>;
    using microseconds = std::chrono::microseconds;

    ThreadLoop(const std::wstring& name);
    ~ThreadLoop();
    void Start(
        const ThreadFunc& func,
        const microseconds& interval = microseconds(1'000'000 / 60));
    void Restart();
    void Stop();
    void SetInitializer(const ThreadFunc& func) { initializerFunc_ = func; }
    void SetFinalizer(const ThreadFunc& func) { finalizerFunc_ = func; }
    bool IsRunning() const;
    bool HasFunction() const;

private:
    const std::wstring name_;
    std::thread thread_;
    std::atomic<bool> isRunning_ = false;
    microseconds interval_ = microseconds::zero();
    ThreadFunc loopFunc_ = nullptr;
    ThreadFunc finalizerFunc_ = nullptr;
    ThreadFunc initializerFunc_ = nullptr;
};
