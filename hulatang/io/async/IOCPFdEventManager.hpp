#ifndef HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP

#include "hulatang/io/FdEventManager.hpp"

#include "hulatang/base/platform/win32/Type.hpp"

namespace hulatang::io {
class IOCPFdEventManager : public FdEventManager
{
public:
    explicit IOCPFdEventManager(EventLoop *loop);
    ~IOCPFdEventManager() override;

    void process(microseconds blockTime) override;

    void add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &descriptor) override;
    void cancel(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd) override;

    void wakeup() override;

private:
    HANDLE iocpHandle;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
