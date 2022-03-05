#ifndef HULATANG_IO_FDEVENTMANAGER_HPP
#define HULATANG_IO_FDEVENTMANAGER_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include <chrono>
#include <vector>

namespace hulatang::io {
class EventLoop;

class FdEventManager
{
public:
    using microseconds = std::chrono::microseconds;
    virtual void process(microseconds blockTime);

    virtual void add(const FdEventWatcherPtr &watcher, const base::FileDescriptor& fd);
    virtual void cancel(const FdEventWatcherPtr &watcher);

protected:
    EventLoop *loop;
    std::vector<FdEventWatcherPtr> watchers;
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTMANAGER_HPP
