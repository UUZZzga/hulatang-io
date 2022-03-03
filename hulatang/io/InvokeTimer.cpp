#include "hulatang/io/InvokeTimer.hpp"

namespace hulatang::io {

InvokeTimerPtr InvokeTimer::create(EventLoop *loop, TimerEventWatcherPtr watcher)
{
    InvokeTimerPtr invokeTimer = std::make_shared<InvokeTimer>(loop, watcher);
    return invokeTimer;
}

InvokeTimer::InvokeTimer(EventLoop *_loop, TimerEventWatcherPtr _watcher)
    : loop(_loop)
    , watcher(_watcher)
{
    _watcher->setCancelCallback([this] { onCancel(); });
}

void InvokeTimer::cancel()
{
    auto timerEventWatcher = watcher.lock();
    if (timerEventWatcher)
    {
        timerEventWatcher->cancel();
    }
}

void InvokeTimer::onCancel() {

}

} // namespace hulatang::io