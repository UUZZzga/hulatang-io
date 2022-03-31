#ifndef HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP

#include "hulatang/io/FdEventManager.hpp"

#include "Config.h"

#if !(defined(HAVE_EPOLL_CREATE1) || defined(HAVE_EPOLL_CREATE1))
#    error "Not Supported"
#endif

#include <sys/epoll.h>

namespace hulatang::io {
class EPollFdEventManager : public FdEventManager
{
public:
    explicit EPollFdEventManager(EventLoop *loop);
    ~EPollFdEventManager();

    void process(microseconds blockTime) override;

    void add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &descriptor) override;
    void cancel(const FdEventWatcherPtr &watcher) override;

    void wakeup() override;

private:
    void processEvent(int numEvents);

private:
    int epollFd;
    using EventList = std::vector<struct epoll_event>;
    EventList events;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
