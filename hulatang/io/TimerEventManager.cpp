#include "hulatang/io/EventLoop.hpp"

using std::chrono::operator""ms;
namespace hulatang::io {

void TimerEventManager::process() {}

bool TimerEventManager::isIdle() const noexcept
{
    return true;
}

std::chrono::microseconds TimerEventManager::nextTriggerTime() const noexcept
{
    return 0ms;
}

} // namespace hulatang::io