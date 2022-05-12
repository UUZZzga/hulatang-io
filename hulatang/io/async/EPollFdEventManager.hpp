#ifndef HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP

#include "hulatang/io/async/LinuxFdEventManager.hpp"
#include <vector>

#if !(defined(HAVE_EPOLL_CREATE) || defined(HAVE_EPOLL_CREATE1))
#    error "Not Supported"
#endif

#include <sys/epoll.h>

namespace hulatang::io {
class EPollFdEventManager : public LinuxFdEventManager
{
public:
    explicit EPollFdEventManager(EventLoop *loop);
    ~EPollFdEventManager() override;

    void process(microseconds blockTime) override;

    void add(Channel *channel) override;
    void update(Channel *channel) override;
    void cancel(Channel *channel) override;

    void wakeup() override;

private:
    void processEvent(int numEvents);

    int epollFd;
    using EventList = std::vector<struct epoll_event>;
    EventList events;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
