#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/base/Buf.hpp"

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop, const FdEventWatcherPtr &watcher)
    : loop(_loop)
    , watcherWPtr(watcher)
{}

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
    channel->enableReading();
    connectionCallback(conn);
}

void TCPConnection::connectDestroyed() {}
} // namespace hulatang::io