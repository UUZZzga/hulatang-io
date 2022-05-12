#include "hulatang/io/async/EPollFdEventManager.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/extend/Cast.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include <chrono>
#include <cstdint>
#include <sys/epoll.h>
#include <unistd.h>

#include <sys/resource.h>

template class std::vector<struct epoll_event>;

constexpr uintptr_t WAKEUP_NUMBER = 1;

namespace hulatang::io {

std::unique_ptr<FdEventManager> FdEventManager::create(EventLoop *loop)
{
    return std::make_unique<EPollFdEventManager>(loop);
}

using base::FileErrorCode;
using base::make_file_error_condition;

EPollFdEventManager::EPollFdEventManager(EventLoop *loop)
    : LinuxFdEventManager(loop)
#ifdef HAVE_EPOLL_CREATE1
    , epollFd(epoll_create1(EPOLL_CLOEXEC))
#else
    , epollFd(epoll_create(1024))
#endif
    , events(128)
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
    startProcess();
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    milliseconds timeout = duration_cast<milliseconds>(blockTime);
    int numEvents = ::epoll_wait(epollFd, &*events.begin(), static_cast<int>(events.size()), static_cast<int>(timeout.count()));
    endProcess();
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

void epollCtl(int epollFd, int op, const Channel *channel)
{
    uint32_t events = 0;
    if (channel->isReading())
    {
        events |= EPOLLIN | EPOLLPRI;
    }
    if (channel->isWriting())
    {
        events |= EPOLLOUT;
    }
    epoll_event event{.events = events, .data = {.ptr = const_cast<Channel *>(channel)}};
    if (::epoll_ctl(epollFd, op, channel->getFd().getFd(), &event) < 0)
    {
        int err = errno;
        HLT_CORE_ERROR("error code: {}", err);
        abort();
    }
}

void EPollFdEventManager::add(Channel *channel)
{
    FdEventManager::add(channel);
    epollCtl(epollFd, EPOLL_CTL_ADD, channel);
}

void EPollFdEventManager::update(Channel *channel)
{
    HLT_CORE_TRACE("EPollFdEventManager::update fd: {}", channel->getFd().getFd());
    epollCtl(epollFd, EPOLL_CTL_MOD, channel);
}

void EPollFdEventManager::cancel(Channel *channel)
{
    HLT_CORE_TRACE("EPollFdEventManager::cancel");
    HLT_CORE_DEBUG("fd: {}", channel->getFd().getFd());
    epollCtl(epollFd, EPOLL_CTL_DEL, channel);
    FdEventManager::cancel(channel);
}
// void EPollFdEventManager::add(Channel *channel)
// {
//     FdEventManager::add(channel);
//     if (!channel->isNoneEvent())
//     {
//         epollCtl(epollFd, EPOLL_CTL_ADD, channel);
//     }
//     else
//     {
//         auto index = channel->getFd().getFd();
//         channels[index] = reinterpret_cast<Channel *>(reinterpret_cast<uintptr_t>(channels[index]) | 1);
//     }
// }

// void EPollFdEventManager::update(Channel *channel)
// {
//     HLT_CORE_TRACE("EPollFdEventManager::update fd: {}", channel->getFd().getFd());
//     auto index = channel->getFd().getFd();
//     bool add = reinterpret_cast<uintptr_t>(channels[index]) & 1;
//     if (channel->isNoneEvent())
//     {
//         if (!add)
//         {
//             epollCtl(epollFd, EPOLL_CTL_DEL, channel);
//             channels[index] = reinterpret_cast<Channel *>(reinterpret_cast<uintptr_t>(channels[index]) | 1);
//         }
//     }
//     else
//     {
//         if (add)
//         {
//             epollCtl(epollFd, EPOLL_CTL_ADD, channel);
//             channels[index] = const_cast<Channel *>(channel);
//         }
//         else
//         {
//             epollCtl(epollFd, EPOLL_CTL_MOD, channel);
//         }
//     }
// }

// void EPollFdEventManager::cancel(Channel *channel)
// {
//     HLT_CORE_TRACE("EPollFdEventManager::cancel");
//     HLT_CORE_DEBUG("fd: {}", channel->getFd().getFd());
//     auto index = channel->getFd().getFd();
//     bool add = reinterpret_cast<uintptr_t>(channels[index]) & 1;
//     if(!add)
//     {
//         epollCtl(epollFd, EPOLL_CTL_DEL, channel);
//     }
//     FdEventManager::cancel(channel);
// }

void EPollFdEventManager::wakeup() {}

void EPollFdEventManager::processEvent(int numEvents)
{
    assert(implicit_cast<size_t>(numEvents) <= events.size());
    for (size_t i = 0; i < numEvents; i++)
    {
        auto &event = events[i];
        if (event.events == 0)
        {
            continue;
        }
        auto *channel = static_cast<Channel *>(event.data.ptr);
        assert(channel != nullptr);
        channel->setREvent(event.events);
        // activeChannels.push_back(channel);
        channel->handler();
    }
}
} // namespace hulatang::io