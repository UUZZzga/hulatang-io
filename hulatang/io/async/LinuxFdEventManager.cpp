#include "hulatang/io/async/LinuxFdEventManager.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/extend/Cast.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include "hulatang/io/SocketChannel.hpp"
#include <chrono>
#include <sys/eventfd.h>
#include <unistd.h>

namespace hulatang::io {
LinuxFdEventManager::LinuxFdEventManager(EventLoop *loop)
    : FdEventManager(loop)
    , eventFd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
{}

LinuxFdEventManager::~LinuxFdEventManager()
{
    eventFd.close();
}

void LinuxFdEventManager::init()
{
    FdEventWatcherPtr watcher = std::make_shared<FdEventWatcher>(loop);
    watcher->setReadHandler([](char *buf, size_t size) {
        assert(size == sizeof(uint64_t));
        assert(*reinterpret_cast<uint64_t *>(buf) == 1);
    });
    std::error_condition condition;
    eventFd.read({reinterpret_cast<char *>(&eventBuf), sizeof(eventBuf)}, condition);
    add(watcher, eventFd);
}

void LinuxFdEventManager::wakeup()
{
    eventfd_write(eventFd.getFd(), 0);
}
} // namespace hulatang::io