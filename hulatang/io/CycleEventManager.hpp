#ifndef HULATANG_IO_CYCLEEVENTMANAGER_HPP
#define HULATANG_IO_CYCLEEVENTMANAGER_HPP

#include "hulatang/io/CycleEventWatcher.hpp"

#include <atomic>
#include <concurrentqueue/concurrentqueue.h>
#include <functional>
#include <memory>

namespace hulatang::io {
class CycleEventManager
{
public:
    typedef moodycamel::ConcurrentQueue<CycleEventWatcher> ConcurrentQueue;
    typedef std::unique_ptr<ConcurrentQueue> ConcurrentQueuePtr;

public:
    CycleEventManager();

    void process();

    [[nodiscard]] bool isIdle() const noexcept;

    void createEvent(const std::function<void()>& f);
    void createEvent(std::function<void()>&& f);

private:
    ConcurrentQueuePtr queue;
    std::atomic_int32_t size;
};
} // namespace hulatang::io

#endif // HULATANG_IO_CYCLEEVENTMANAGER_HPP
