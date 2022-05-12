#include "hulatang/io/async/LinuxFdEventManager.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/extend/Cast.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include <chrono>
#include <memory>
#include <sys/eventfd.h>
#include <sys/poll.h>
#include <unistd.h>

namespace hulatang::io {
LinuxFdEventManager::LinuxFdEventManager(EventLoop *loop)
    : FdEventManager(loop)
    , eventFd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
    , processing(false)
{}

LinuxFdEventManager::~LinuxFdEventManager()
{
    channel->getFd().close();
}

void LinuxFdEventManager::init()
{
    channel = std::make_unique<Channel>(loop, std::move(eventFd));
    channel->setHandlerCallback([](Channel *channel) {
        if ((channel->getREvent() & POLLIN) != 0)
        {
            eventfd_t event = 0;
            eventfd_read(channel->getFd().getFd(), &event);
        }
    });
    add(channel.get());
    channel->enableReading();
}

void LinuxFdEventManager::wakeup()
{
    if (processing)
    {
        eventfd_write(eventFd.getFd(), 0);
    }
}
} // namespace hulatang::io