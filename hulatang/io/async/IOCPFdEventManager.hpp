#ifndef HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP

#include "hulatang/io/Channel.hpp"
#include "hulatang/io/FdEventManager.hpp"

#include "hulatang/base/platform/win32/Type.hpp"

#include <unordered_set>
#include <vector>

namespace hulatang::io {
class IOCPFdEventManager : public FdEventManager
{
public:
    explicit IOCPFdEventManager(EventLoop *loop);
    ~IOCPFdEventManager() override;

    void process(microseconds blockTime) override;

    void add(Channel *channel) override;
    void update(Channel *channel) override;
    void cancel(Channel *channel) override;

    void wakeup() override;

private:
    HANDLE iocpHandle;
    std::vector<OVERLAPPED_ENTRY> completionPortEntries;
    std::unordered_set<Channel *> currentChannels;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
