#ifndef HULATANG_IO_TIMEREVENTMANAGER_HPP
#define HULATANG_IO_TIMEREVENTMANAGER_HPP

#include "hulatang/io/TimerEventWatcher.hpp"

#include <chrono>
#include <memory>
#include <set>

namespace hulatang::io {
class EventLoop;

class TimerEventManager
{
public:
    using TimerEventWatcherPtr = std::shared_ptr<TimerEventWatcher>;
    using microseconds = std::chrono::microseconds;

public:
    explicit TimerEventManager(EventLoop *loop);

    void process();

    [[nodiscard]] bool isIdle() const noexcept;

    [[nodiscard]] microseconds nextTriggerTime() const noexcept;

    void add(const TimerEventWatcherPtr &watcher);

    void cancel(const TimerEventWatcherPtr &watcher);

private:
    struct LessTimerEventWatcher
    {
        [[nodiscard]] bool operator()(const TimerEventWatcherPtr &lhs, const TimerEventWatcherPtr &rhs) const
        {
            return lhs->getTimeout() < rhs->getTimeout();
        }
    };
    EventLoop *loop;
    std::set<TimerEventWatcherPtr, LessTimerEventWatcher> timerSet;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TIMEREVENTMANAGER_HPP
