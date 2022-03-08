#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/base/Buf.hpp"

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop, const FdEventWatcherPtr &watcher)
    : loop(_loop)
    , watcherWPtr(watcher)
    , state(kConnecting)
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
    assert(state == kConnecting);
    setState(kConnected);

    assert(!watcherWPtr.expired());
    auto conn = shared_from_this();
    FdEventWatcherPtr watcher = watcherWPtr.lock();
    channel = std::make_shared<SocketChannel>(loop, fd, watcher);
    channel->setConnectionCallback([_this = conn](auto &) { _this->connectDestroyed(); });
    channel->setMessageCallback([_this = conn](const base::Buf &buf) { _this->messageCallback(_this, buf); });

    watcher->setReadHandler([_this = channel](char *buf, size_t n) { _this->recvByteNum(buf, n); });
    watcher->setWriteHandler([_this = channel](size_t n) { _this->sendByteNum(n); });
    watcher->setCloseHandler([_this = channel] { _this->close(); });
    channel->enableReading();
    connectionCallback(conn);
}

void TCPConnection::connectDestroyed()
{
    loop->assertInLoopThread();
    if (state == kConnected)
    {
        setState(kDisconnected);
        channel->disableAll();

        connectionCallback(shared_from_this());
    }
    // channel->remove();
}
} // namespace hulatang::io