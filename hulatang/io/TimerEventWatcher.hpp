#ifndef HULATANG_IO_TIMEREVENTWATCHER_HPP
#define HULATANG_IO_TIMEREVENTWATCHER_HPP

#include "hulatang/io/EventWatcher.hpp"

#include <chrono>
#include <memory>

namespace hulatang::io {
class TimerEventManager;
class TimerEventWatcher;
typedef std::shared_ptr<TimerEventWatcher> TimerEventWatcherPtr;

class TimerEventWatcher : public std::enable_shared_from_this<TimerEventWatcher>, public ClosableEventWatcher
{
public:
    using microseconds = std::chrono::microseconds;

public:
    TimerEventWatcher(EventLoop *loop, microseconds _timeout);
    ~TimerEventWatcher() override;

    static TimerEventWatcherPtr create(EventLoop *loop, microseconds timeout, std::function<void()> &&f);
    static TimerEventWatcherPtr create(EventLoop *loop, microseconds timeout, const std::function<void()> &f);

    virtual void handle();

    microseconds getTimeout() const
    {
        return timeout;
    }

protected:
    void doCancel() override;

protected:
    microseconds timeout;
    bool attached;
};

class RepeatTimerEventWatcher : public TimerEventWatcher
{
public:
    static constexpr uint32_t Infinite = UINT32_MAX;
    static TimerEventWatcherPtr create(EventLoop *loop, microseconds interval, uint32_t repeat, std::function<void()> &&f);
    static TimerEventWatcherPtr create(EventLoop *loop, microseconds interval, uint32_t repeat, const std::function<void()> &f);

    RepeatTimerEventWatcher(EventLoop *loop, microseconds interval, uint32_t repeat);

    void handle() override;

private:
    microseconds interval;
    uint32_t repeat;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TIMEREVENTWATCHER_HPP
