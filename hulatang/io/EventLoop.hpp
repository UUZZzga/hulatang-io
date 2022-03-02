#ifndef HULATANG_IO_EVENTLOOP_HPP
#define HULATANG_IO_EVENTLOOP_HPP

#include "hulatang/io/CycleEventManager.hpp"
#include "hulatang/io/IdleEventManager.hpp"
#include "hulatang/io/Status.hpp"
#include "hulatang/io/TimerEventManager.hpp"
#include "hulatang/io/FdEventManager.hpp"

#include <functional>
#include <thread>
#include <cassert>
#include <chrono>

namespace hulatang::io {
class EventLoop
{
public:
    typedef std::chrono::microseconds microseconds;

public:
    EventLoop();

    void run();

    void stop();

    void runInLoop(const std::function<void()>& f);
    void queueInLoop(const std::function<void()>& f);

public:
    bool isInLoopThread() const noexcept
    {
        return std::this_thread::get_id() == tid;
    }

    void assertInLoopThread() const noexcept
    {
        assert(isInLoopThread());
    }

private:
    void init();
    void initNotifyPipeWatcher();
    void freeNotifyPipeWatcher();

    microseconds calculationBlocktime() noexcept;

    void runOnce();
    void updateTime();

    bool isIdle() const noexcept;

    void processIo(microseconds blockTime);
    void processTimerAndTimeout();
    void processIdle();
    void processCycle();

private:
    microseconds currentTime;
    TimerEventManager timerEventManager;
    FdEventManager fdEventManager;
    CycleEventManager cycleEventManager;
    IdleEventManager idleEventManager;
    std::thread::id tid;
    status::EventLoopStatus status;
};
} // namespace hulatang::io
#endif // HULATANG_IO_EVENTLOOP_HPP
