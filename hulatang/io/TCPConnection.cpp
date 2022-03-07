#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/base/Buf.hpp"

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop, const FdEventWatcherPtr &watcher)
    : loop(_loop)
    , watcherWPtr(watcher)
{}

void TCPConnection::send(const base::Buf &buf)
{
    int expected = 0;
    int desired = 1;
    if (!sending.compare_exchange_strong(expected, desired))
    {
        abort();
    }
    std::error_condition condition;
    channel->getFd().write(buf, condition);
    if (condition)
    {
        HLT_CORE_WARN("condition code{}, message{}", condition.value(), condition.message());
    }
    sending.compare_exchange_strong(desired, expected);
}

void TCPConnection::shutdown() {}

void TCPConnection::connectEstablished(base::FileDescriptor &fd)
{
    loop->assertInLoopThread();
    assert(!watcherWPtr.expired());
    auto conn = shared_from_this();
    FdEventWatcherPtr watcher = watcherWPtr.lock();
    channel = std::make_shared<SocketChannel>(loop, fd, watcher);
    channel->setMessageCallback([_this = conn](const base::Buf &buf) { _this->messageCallback(_this, buf); });
    watcher->setReadHandler([_this = channel](char *buf, size_t n) { _this->recvByteNum(buf, n); });
    watcher->setWriteHandler([_this = channel](size_t n) { _this->sendByteNum(n); });
    channel->enableReading();
    connectionCallback(conn);
}

void TCPConnection::connectDestroyed() {}
} // namespace hulatang::io