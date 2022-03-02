#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/Status.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/String.hpp"

#include <chrono>
#include <thread>

using std::chrono::microseconds;
using std::chrono::operator""ms;
using std::chrono::operator""us;

constexpr microseconds MaximumBlockingTime = 100ms;

namespace {
thread_local hulatang::io::EventLoop *thisEventLoop = nullptr;
}

namespace hulatang::io {
using status::EnumEventLoopStatus;
EventLoop::EventLoop()
{
    init();
}

void EventLoop::run()
{
    DLOG_TRACE;
    assertInLoopThread();
    status.store(EnumEventLoopStatus::Starting);

    status.store(EnumEventLoopStatus::Running);
    while (status == EnumEventLoopStatus::Running)
    {
        runOnce();
    }
    freeNotifyPipeWatcher();
    DLOG_TRACE_ARG("EventLoop stopped, tid={}", base::to_string(tid));
    status.store(EnumEventLoopStatus::Stopped);
}

void EventLoop::stop()
{
    DLOG_TRACE;
}

void EventLoop::queueInLoop(const std::function<void()> &f)
{
    cycleEventManager.createEvent(f);
}

void EventLoop::runInLoop(const std::function<void()> &f)
{
    if (isInLoopThread())
    {
        f();
    }
    else
    {
        queueInLoop(f);
    }
}

void EventLoop::init()
{
    DLOG_TRACE;
    status.store(EnumEventLoopStatus::Initializing);
    tid = std::this_thread::get_id();
    initNotifyPipeWatcher();
    status.store(EnumEventLoopStatus::Initialized);
}

void EventLoop::initNotifyPipeWatcher()
{
    DLOG_TRACE;
    // TODO: Implement
}

void EventLoop::freeNotifyPipeWatcher()
{
    DLOG_TRACE;
    // TODO: Implement
}

microseconds EventLoop::calculationBlocktime() noexcept
{
    if (isIdle())
    {
        return MaximumBlockingTime;
    }
    microseconds next_timeout = timerEventManager.nextTriggerTime();
    if (next_timeout < 0ms)
    {
        timerEventManager.process();
        updateTime();
    }
    if (!cycleEventManager.isIdle())
    {
        return 0ms;
    }
    if (isIdle())
    {
        return MaximumBlockingTime;
    }
    next_timeout = timerEventManager.nextTriggerTime();
    if (next_timeout < 1ms)
    {
        next_timeout = 1ms;
    }
    return min(MaximumBlockingTime, next_timeout);
}

void EventLoop::runOnce()
{
    updateTime();
    microseconds blocktime = calculationBlocktime();
    processIo(blocktime);
    if (blocktime > 500us)
    {
        updateTime();
    }
    processTimerAndTimeout();
    if (isIdle())
    {
        processIdle();
    }
    else
    {
        processCycle();
    }
}

void EventLoop::updateTime()
{
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    using std::chrono::system_clock;
    currentTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());
}

bool EventLoop::isIdle() const noexcept
{
    return timerEventManager.isIdle() && cycleEventManager.isIdle();
}

void EventLoop::processIo(microseconds blockTime)
{
    fdEventManager.process(blockTime);
}

void EventLoop::processTimerAndTimeout()
{
    timerEventManager.process();
}

void EventLoop::processIdle()
{
    idleEventManager.process();
}

void EventLoop::processCycle()
{
    cycleEventManager.process();
}

} // namespace hulatang::io