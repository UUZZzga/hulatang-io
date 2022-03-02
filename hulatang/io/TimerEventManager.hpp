#ifndef HULATANG_IO_TIMEREVENTMANAGER_HPP
#define HULATANG_IO_TIMEREVENTMANAGER_HPP

#include <chrono>
#include <set>

namespace hulatang::io {
class TimerEventWatcher;
class TimerEventManager
{
public:
    void process();

    [[nodiscard]] bool isIdle() const noexcept;

    std::chrono::microseconds nextTriggerTime() const noexcept;

private:
    std::set<TimerEventWatcher *> timerSet;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TIMEREVENTMANAGER_HPP
