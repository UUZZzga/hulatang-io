#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/InvokeTimer.hpp"
#include "hulatang/io/Status.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/String.hpp"

#include "hulatang/io/NetInit.hpp"

#include <chrono>
#include <memory>
#include <thread>

using microseconds = hulatang::io::EventLoop::microseconds;
using std::chrono::operator""ms;
using std::chrono::operator""us;

constexpr microseconds MaximumBlockingTime = 100ms;

namespace {
thread_local hulatang::io::EventLoop *currentEventLoop = nullptr;
}

namespace hulatang::io {
using status::EnumEventLoopStatus;
EventLoop::EventLoop()
    : currentTime(0)
    , timerEventManager(this)
{
    NetInit::init();
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
    status.store(EnumEventLoopStatus::Stopping);
    wakeup();
}

void EventLoop::queueInLoop(const std::function<void()> &f)
{
    cycleEventManager.createEvent(f);
}

InvokeTimerPtr EventLoop::runAfter(microseconds time, const std::function<void()> &f)
{
    auto watcher = TimerEventWatcher::create(this, time + currentTime, f);
    return InvokeTimer::create(this, watcher);
}

InvokeTimerPtr EventLoop::runEvery(microseconds interval, const std::function<void()> &f, uint32_t repeat)
{
    auto watcher = RepeatTimerEventWatcher::create(this, interval, repeat, f);
    return InvokeTimer::create(this, watcher);
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
    fdEventManager = FdEventManager::create(this);
    fdEventManager->init();
    tid = std::this_thread::get_id();
    initNotifyPipeWatcher();
    updateTime();
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
        next_timeout = timerEventManager.nextTriggerTime();
    }
    if (!cycleEventManager.isIdle())
    {
        return 0ms;
    }
    if (isIdle())
    {
        return MaximumBlockingTime;
    }
    if (next_timeout < 1ms)
    {
        next_timeout = 1ms;
    }
    return min(MaximumBlockingTime, next_timeout);
}

void EventLoop::runOnce()
{
    DLOG_TRACE;
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
    using std::chrono::system_clock;
    currentTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return currentEventLoop;
}

bool EventLoop::isIdle() const noexcept
{
    return timerEventManager.isIdle() && cycleEventManager.isIdle();
}

void EventLoop::processIo(microseconds blockTime)
{
    DLOG_TRACE;
    fdEventManager->process(blockTime);
}

void EventLoop::processTimerAndTimeout()
{
    DLOG_TRACE;
    timerEventManager.process();
}

void EventLoop::processIdle()
{
    DLOG_TRACE;
    idleEventManager.process();
}

void EventLoop::processCycle()
{
    DLOG_TRACE;
    cycleEventManager.process();
}

void EventLoop::wakeup()
{
    DLOG_TRACE;
    fdEventManager->wakeup();
}

} // namespace hulatang::io