#ifndef HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP

#include "Config.h"

#include "hulatang/io/FdEventWatcher.hpp"
#include "hulatang/io/FdEventManager.hpp"
#include <atomic>
#include <vector>

namespace hulatang::io {
class LinuxFdEventManager : public FdEventManager
{
public:
    explicit LinuxFdEventManager(EventLoop *loop);
    ~LinuxFdEventManager() override;

    void init() override;

    void wakeup() override;

protected:
    void startProcess()
    {
        processing = true;
    }

    void endProcess()
    {
        processing = false;
    }

private:
    base::FileDescriptor eventFd;
    uint64_t eventBuf;
    std::atomic<bool> processing;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP
