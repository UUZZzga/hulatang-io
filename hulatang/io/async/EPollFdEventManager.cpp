#include "hulatang/io/async/EPollFdEventManager.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/extend/Cast.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include "hulatang/io/SocketChannel.hpp"
#include <chrono>
#include <sys/epoll.h>
#include <unistd.h>

constexpr uintptr_t WAKEUP_NUMBER = 1;

namespace hulatang::io {

std::unique_ptr<FdEventManager> FdEventManager::create(EventLoop *loop)
{
    return std::make_unique<EPollFdEventManager>(loop);
}

using base::FileErrorCode;
using base::make_file_error_condition;

EPollFdEventManager::EPollFdEventManager(EventLoop *loop)
    : FdEventManager(loop)
#ifdef HAVE_EPOLL_CREATE1
    , epollFd(epoll_create1(EPOLL_CLOEXEC))
#else
    , epollFd(epoll_create(1024))
#endif
{
#ifndef HAVE_EPOLL_CREATE1
    int val = fcntl(epollFd, F_GETFD);
    val |= FD_CLOEXEC;
    fcntl(epollFd, F_SETFD, val);
#endif
    if (epollFd < 0)
    {
        HLT_CORE_ERROR("EPollFdEventManager::EPollFdEventManager");
    }
}

EPollFdEventManager::~EPollFdEventManager()
{
    ::close(epollFd);
}

void EPollFdEventManager::process(microseconds blockTime)
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    milliseconds timeout = duration_cast<milliseconds>(blockTime);
    int numEvents = ::epoll_wait(epollFd, &*events.begin(), static_cast<int>(events.size()), static_cast<int>(timeout.count()));
    if (numEvents > 0)
    {
        HLT_CORE_TRACE("{} events happened", numEvents);
        processEvent(numEvents);
        if (numEvents == events.size())
        {
            events.resize(events.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        HLT_CORE_TRACE("nothing happened");
    }
    else
    {
        int savedErrno = errno;
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            HLT_CORE_ERROR("EPollPoller::poll()");
        }
    }
}

void EPollFdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &descriptor)
{
    FdEventManager::add(watcher, descriptor);
    // CreateIoCompletionPort(reinterpret_cast<HANDLE>(descriptor.getFd()), iocpHandle, reinterpret_cast<ULONG_PTR>(watcher.get()), 0);
}

void EPollFdEventManager::cancel(const FdEventWatcherPtr &watcher)
{
    FdEventManager::cancel(watcher);
    // disableAll();
}

void EPollFdEventManager::wakeup()
{

}

void EPollFdEventManager::processEvent(int numEvents)
{
    assert(implicit_cast<size_t>(numEvents) <= events.size());
    size_t psize = 0;
    for (auto &event : events)
    {
        if (event.events == 0)
        {
            continue;
        }
        auto *impl = static_cast<base::FileDescriptor::Impl *>(event.data.ptr);
        auto *watcher = static_cast<FdEventWatcher *>(impl->watcher);
        if ((event.events & (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLPRI)) != 0)
        {
            // read event
            if (impl->accept != 1U)
            {
                read(impl->fd, impl->readBuf.buf, impl->readBuf.len);
            }
            watcher->readHandle(impl->readBuf.buf, impl->readBuf.len);
        }
        if ((event.events & (EPOLLOUT | EPOLLET | EPOLLHUP)) != 0U)
        {
            // write event
            ssize_t size = write(impl->fd, impl->writeBuf.buf, impl->writeBuf.len);
            if (size >= 0)
            {
                watcher->writeHandle(size);
            }
        }
        psize++;
        if (psize == implicit_cast<size_t>(numEvents))
        {
            break;
        }
    }
}
} // namespace hulatang::io