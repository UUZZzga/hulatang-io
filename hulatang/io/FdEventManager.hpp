#ifndef HULATANG_IO_FDEVENTMANAGER_HPP
#define HULATANG_IO_FDEVENTMANAGER_HPP

#include "hulatang/io/Channel.hpp"
#include <chrono>
#include <memory>
#include <vector>

namespace hulatang::io {
class EventLoop;

class FdEventManager
{
public:
    using microseconds = std::chrono::microseconds;

    explicit FdEventManager(EventLoop *loop);
    virtual ~FdEventManager() = default;

    virtual void init() {}

    virtual void process(microseconds blockTime);

    virtual void add(Channel *channel);
    virtual void update(Channel *channel);
    virtual void cancel(Channel *channel);

    static std::unique_ptr<FdEventManager> create(EventLoop *loop);

    virtual void wakeup();

protected:
    EventLoop *loop;
    std::vector<Channel *> channels;
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTMANAGER_HPP
