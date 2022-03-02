#include "hulatang/io/CycleEventManager.hpp"

namespace hulatang::io {

CycleEventManager::CycleEventManager()
    : queue(std::make_unique<ConcurrentQueue>())
    , size(0)
{
}

void CycleEventManager::process()
{
    int32_t eventCount = size;
    for (int32_t i = 0; i < eventCount; ++i)
    {
        CycleEventWatcher event;
        while(!queue->try_dequeue(event)){}
        event.handle();
        --size;
    }
}

bool CycleEventManager::isIdle() const noexcept
{
    return size == 0;
}

void CycleEventManager::createEvent(const std::function<void()>& f) {
    CycleEventWatcher watcher;
    watcher.setHandler(f);
    while (!queue->try_enqueue(watcher)) {}
    ++size;
}

void CycleEventManager::createEvent(std::function<void()>&& f) {
    CycleEventWatcher watcher;
    watcher.setHandler(move(f));
    while (!queue->try_enqueue(watcher)) {}
    ++size;
}

} // namespace hulatang::io