#ifndef HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP

#include "Config.h"

#include "hulatang/io/FdEventManager.hpp"
#include <atomic>
#include <memory>
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
    std::unique_ptr<Channel> channel;
    std::atomic<bool> processing;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_LINUXFDEVENTMANAGER_HPP
