#include "hulatang/io/TimerEventManager.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"

using std::chrono::operator""ms;
namespace hulatang::io {
using microseconds = TimerEventManager::microseconds;

TimerEventManager::TimerEventManager(EventLoop *_loop)
    : loop(_loop)
{}

void TimerEventManager::process()
{
    DLOG_TRACE;
    microseconds currentTime = loop->getCurrentTime();
    for (auto it = timerSet.begin(), end = timerSet.end(); it != end;)
    {
        TimerEventWatcherPtr timer = *it;
        if (timer->getTimeout() <= currentTime)
        {
            it = timerSet.erase(it);
            timer->handle();
        }
        else
        {
            break;
        }
    }
}

bool TimerEventManager::isIdle() const noexcept
{
    return timerSet.empty();
}

microseconds TimerEventManager::nextTriggerTime() const noexcept
{
    if (timerSet.empty())
    {
        return 1000ms;
    }
    return (*timerSet.begin())->getTimeout() - loop->getCurrentTime();
}

void TimerEventManager::add(const TimerEventWatcherPtr &watcher)
{
    DLOG_TRACE;
    timerSet.emplace(watcher);
}

void TimerEventManager::cancel(const TimerEventWatcherPtr &watcher)
{
    DLOG_TRACE;
    timerSet.erase(watcher);
}

} // namespace hulatang::io