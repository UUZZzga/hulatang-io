#ifndef HULATANG_IO_INVOKETIMER_HPP
#define HULATANG_IO_INVOKETIMER_HPP

#include "hulatang/io/TimerEventWatcher.hpp"

#include <memory>

namespace hulatang::io {
class InvokeTimer;
using InvokeTimerPtr = std::shared_ptr<InvokeTimer>;

class InvokeTimer : public std::enable_shared_from_this<InvokeTimer>
{
public:
    static InvokeTimerPtr create(EventLoop *loop, TimerEventWatcherPtr watcher);

    InvokeTimer(EventLoop *loop, TimerEventWatcherPtr watcher);

    void cancel();

private:
    void onCancel();

private:
    EventLoop *loop;
    TimerEventWatcherPtr::weak_type watcher;
};
} // namespace hulatang::io

#endif // HULATANG_IO_INVOKETIMER_HPP
