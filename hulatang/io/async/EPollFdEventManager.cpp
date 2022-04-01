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
    , events(128)
    , watchers(128)
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
        if (implicit_cast<size_t>(numEvents) == events.size())
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

void EPollFdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd)
{
    FdEventManager::add(watcher, fd);
    auto fdNum = fd.getFd();

    auto *impl = const_cast<base::FileDescriptor::Impl *>(fd.getImpl());
    uint32_t events = 0;
    if (impl->accept)
    {
        events |= EPOLLIN;
    }
    else
    {
        if (impl->read)
        {
            events |= EPOLLIN | EPOLLPRI;
        }
        if (impl->write)
        {
            events |= EPOLLOUT;
        }
    }
    epoll_event event{.events = events, .data = {.ptr = impl}};
    auto idx = implicit_cast<size_t>(fdNum);
    int op = watchers[idx] == nullptr ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (::epoll_ctl(epollFd, op, fdNum, &event) < 0)
    {
        int err = errno;
        abort();
    }
    if (watchers.size() < idx)
    {
        watchers.resize(watchers.size() * 2);
    }
    assert(idx < watchers.size());
    watchers[idx] = watcher.get();
}

void EPollFdEventManager::cancel(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd)
{
    FdEventManager::cancel(watcher, fd);
    // disableAll();
    auto idx = implicit_cast<size_t>(fd.getFd());
    assert(idx < watchers.size());
    assert(watchers[idx] != nullptr);
    watchers[idx] = nullptr;
}

void EPollFdEventManager::wakeup() {}

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
        auto *watcher = watchers[impl->fd];
        assert(watcher != nullptr);
        if ((event.events & (EPOLLIN | EPOLLET | EPOLLHUP)) != 0)
        {
            // read event
            ssize_t size;
            if (impl->accept != 1U)
            {
                size = read(impl->fd, impl->readBuf.buf, impl->readBuf.len);
                if (size < 0)
                {
                    int err = errno;
                    switch (err)
                    {
                    case EAGAIN:
#if EWOULDBLOCK != EAGAIN
                    case EWOULDBLOCK:
#endif
                    {
                        // 没有请求
                    }
                    break;

                    case EINVAL:
                    case EBADF:  // fd不是有效的文件描述符或未打开读取
                    case EFAULT: // buf在您可访问的地址空间之外
                    case EINTR:  // 在读取任何数据之前，被信号中断
                    {
                        // TODO
                        abort();
                    }
                    break;
                    }
                }
            }
            watcher->readHandle(impl->readBuf.buf, implicit_cast<size_t>(size));
        }
        if ((event.events & (EPOLLOUT | EPOLLET | EPOLLHUP)) != 0U)
        {
            // write event
            ssize_t size = write(impl->fd, impl->writeBuf.buf, impl->writeBuf.len);
            if (size >= 0)
            {
                watcher->writeHandle(impl->writeBuf.buf, size);
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