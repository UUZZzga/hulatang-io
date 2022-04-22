#ifndef HULATANG_IO_FDEVENTMANAGER_HPP
#define HULATANG_IO_FDEVENTMANAGER_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include <chrono>
#include <memory>
#include <vector>

namespace hulatang::io {
class EventLoop;

class FdEventManager
{
public:
    using microseconds = std::chrono::microseconds;

    explicit FdEventManager(EventLoop *loop);
    virtual ~FdEventManager() = default;

    virtual void init() {}

    virtual void process(microseconds blockTime);

    virtual void add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd);
    virtual void change(const base::FileDescriptor &fd);
    virtual void cancel(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd);

    static std::unique_ptr<FdEventManager> create(EventLoop *loop);

    virtual void wakeup();

protected:
    [[nodiscard]] FdEventWatcherPtr getWatcher(const base::FileDescriptor &fd) const;

    EventLoop *loop;
    std::vector<FdEventWatcherPtr> watchers;
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTMANAGER_HPP
