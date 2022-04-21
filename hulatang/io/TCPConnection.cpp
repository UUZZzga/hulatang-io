#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/base/Buf.hpp"
#include "hulatang/base/Log.hpp"

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
    channel = std::make_shared<SocketChannel>(loop, std::move(fd), watcher, conn);
    channel->setConnectionCallback([this](auto &) { closeCallback(shared_from_this()); });
    channel->setMessageCallback([this](const base::Buf &buf) { messageCallback(shared_from_this(), buf); });

    watcher->setReadHandler([_this = channel.get()](char *buf, size_t n) { _this->recvByteNum(buf, n); });
    watcher->setWriteHandler([_this = channel.get()](char *buf, size_t n) { _this->sendByteNum(buf, n); });
    watcher->setCloseHandler([_this = channel]() mutable {
        _this->close();
        _this.reset();
    });
    channel->enableReading();
    connectionCallback(conn);
}

void TCPConnection::connectDestroyed()
{
    loop->assertInLoopThread();
    HLT_CORE_TRACE("connectDestroyed::connectDestroyed");

    if (state == kConnected)
    {
        setState(kDisconnected);
        // channel->disableAll();

        auto ptr = shared_from_this();
        connectionCallback(ptr);
    }
}
} // namespace hulatang::io