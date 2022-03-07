#include "hulatang/io/TCPConnection.hpp"

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop)
    : loop(_loop)
{}

void TCPConnection::shutdown() {}

void TCPConnection::connectEstablished(base::FileDescriptor &fd)
{
    loop->assertInLoopThread();
    channel = std::make_shared<SocketChannel>(loop, fd);

    channel->enableReading();
    connectionCallback(shared_from_this());
}

void TCPConnection::connectDestroyed() {}
} // namespace hulatang::io