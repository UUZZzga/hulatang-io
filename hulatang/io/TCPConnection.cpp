#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/base/Buf.hpp"

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop, const FdEventWatcherPtr &watcher, InetAddress peerAddr)
    : loop(_loop)
    , watcherWPtr(watcher)
    , peerAddr(std::move(peerAddr))
    , state(kConnecting)
{}

void TCPConnection::send(const base::Buf &buf)
{
    int32_t expected = 0;
    int32_t desired = 1;
    while (!sending.compare_exchange_strong(expected, desired))
    {
        expected = 0;
    }
    channel->send(buf);
    // std::error_condition condition;
    // channel->getFd().write(buf, condition);
    // if (condition)
    // {
    //     HLT_CORE_WARN("condition code{}, message{}", condition.value(), condition.message());
    // }
    sending.store(0);
}

void TCPConnection::shutdown()
{
    loop->queueInLoop([_this = shared_from_this()] { _this->closeCallback(_this); });
}

void TCPConnection::connectEstablished(base::FileDescriptor &&fd)
{
    loop->assertInLoopThread();
    assert(state == kConnecting);
    setState(kConnected);

    assert(!watcherWPtr.expired());
    auto conn = shared_from_this();
    FdEventWatcherPtr watcher = watcherWPtr.lock();
    channel = std::make_shared<SocketChannel>(loop, std::move(fd), watcher);
    channel->setConnectionCallback([_this = conn](auto &) { _this->connectDestroyed(); });
    channel->setMessageCallback([_this = conn](const base::Buf &buf) { _this->messageCallback(_this, buf); });

    watcher->setReadHandler([_this = channel](char *buf, size_t n) { _this->recvByteNum(buf, n); });
    watcher->setWriteHandler([_this = channel](char *buf, size_t n) { _this->sendByteNum(buf, n); });
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

        auto ptr = shared_from_this();
        connectionCallback(ptr);
    }
    // channel->remove();
}
} // namespace hulatang::io