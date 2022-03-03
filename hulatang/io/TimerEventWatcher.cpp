#include "hulatang/io/TimerEventWatcher.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventWatcher.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TimerEventManager.hpp"
#include <algorithm>
#include <memory>

namespace hulatang::io {

TimerEventWatcher::TimerEventWatcher(EventLoop *loop, microseconds _timeout)
    : ClosableEventWatcher(loop)
    , timeout(_timeout)
    , attached(true)
{}

TimerEventWatcher::~TimerEventWatcher()
{
    doClose();
}

TimerEventWatcherPtr TimerEventWatcher::create(EventLoop *loop, microseconds timeout, std::function<void()> &&f)
{
    auto watcher = std::make_shared<TimerEventWatcher>(loop, timeout);
    watcher->setHandler(std::move(f));
    loop->getTimerEventManager().add(watcher);
    return watcher;
}

TimerEventWatcherPtr TimerEventWatcher::create(EventLoop *loop, microseconds timeout, const std::function<void()> &f)
{
    auto watcher = std::make_shared<TimerEventWatcher>(loop, timeout);
    watcher->setHandler(f);
    loop->getTimerEventManager().add(watcher);
    return watcher;
}

void TimerEventWatcher::handle()
{
    DLOG_TRACE;
    handler();
}

void TimerEventWatcher::doCancel()
{
    DLOG_TRACE;
    if (attached)
    {
        loop->getTimerEventManager().cancel(shared_from_this());
        attached = false;
    }
}

TimerEventWatcherPtr RepeatTimerEventWatcher::create(EventLoop *loop, microseconds interval, uint32_t repeat, std::function<void()> &&f)
{
    auto watcher = std::make_shared<RepeatTimerEventWatcher>(loop, interval, repeat);
    watcher->setHandler(std::move(f));
    loop->getTimerEventManager().add(watcher);
    return watcher;
}

TimerEventWatcherPtr RepeatTimerEventWatcher::create(
    EventLoop *loop, microseconds interval, uint32_t repeat, const std::function<void()> &f)
{
    auto watcher = std::make_shared<RepeatTimerEventWatcher>(loop, interval, repeat);
    watcher->setHandler(f);
    loop->getTimerEventManager().add(watcher);
    return watcher;
}

RepeatTimerEventWatcher::RepeatTimerEventWatcher(EventLoop *loop, microseconds _interval, uint32_t repeat)
    : TimerEventWatcher(loop, loop->getCurrentTime() + _interval)
    , interval(_interval)
    , repeat(repeat)
{}

void RepeatTimerEventWatcher::handle()
{
    DLOG_TRACE;
    handler();
    if (repeat == 0)
    {
        return;
    }
    loop->getTimerEventManager().add(shared_from_this());
    timeout = loop->getCurrentTime() + interval;
    if (repeat != Infinite)
    {
        --repeat;
    }
}

} // namespace hulatang::io