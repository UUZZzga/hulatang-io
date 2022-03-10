#ifndef HULATANG_IO_EVENTLOOP_HPP
#define HULATANG_IO_EVENTLOOP_HPP

#include "hulatang/base/def.h"
#include "hulatang/io/CycleEventManager.hpp"
#include "hulatang/io/IdleEventManager.hpp"
#include "hulatang/io/Status.hpp"
#include "hulatang/io/TimerEventManager.hpp"
#include "hulatang/io/FdEventManager.hpp"

#include "hulatang/io/InvokeTimer.hpp"

#include <functional>
#include <memory>
#include <ratio>
#include <stdint.h>
#include <thread>
#include <cassert>
#include <chrono>

namespace hulatang::io {
class EventLoop
{
public:
    using microseconds = std::chrono::microseconds;

public:
    EventLoop();

    void run();

    void stop();

    void runInLoop(const std::function<void()> &f);
    void queueInLoop(const std::function<void()> &f);

    InvokeTimerPtr runAfter(microseconds time, const std::function<void()> &f);
    InvokeTimerPtr runEvery(microseconds interval, const std::function<void()> &f, uint32_t repeat = RepeatTimerEventWatcher::Infinite);

public:
    bool isInLoopThread() const noexcept
    {
        return std::this_thread::get_id() == tid;
    }

    void assertInLoopThread() const noexcept
    {
        assert(isInLoopThread());
    }

    microseconds getCurrentTime() const
    {
        return currentTime;
    }

    TimerEventManager &getTimerEventManager()
    {
        return timerEventManager;
    }

    void updateTime();

    FdEventManager& getFdEventManager() { return *fdEventManager; }

private:
    void init();
    void initNotifyPipeWatcher();
    void freeNotifyPipeWatcher();

    microseconds calculationBlocktime() noexcept;

    void runOnce();

    bool isIdle() const noexcept;

    void processIo(microseconds blockTime);
    void processTimerAndTimeout();
    void processIdle();
    void processCycle();

    void wakeup();

private:
    microseconds currentTime;
    std::unique_ptr<FdEventManager> fdEventManager;
    TimerEventManager timerEventManager;
    CycleEventManager cycleEventManager;
    IdleEventManager idleEventManager;
    std::thread::id tid;
    status::EventLoopStatus status;
};
} // namespace hulatang::io
#endif // HULATANG_IO_EVENTLOOP_HPP
