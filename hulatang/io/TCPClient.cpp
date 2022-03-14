#include "hulatang/io/TCPClient.hpp"

#include "hulatang/io/TCPConnection.hpp"
#include <memory>

namespace hulatang::io {

TCPClient::TCPClient(EventLoop *_loop, InetAddress _address)
    : loop(_loop)
    , address(std::move(_address))
    , connector(std::make_shared<Connector>(_loop, address))
{
    connector->setNewConnectionCallback([&](base::FileDescriptor &&fd, FdEventWatcherPtr watcher) { newConnection(std::move(fd), watcher); });
}

void TCPClient::connect()
{
    HLT_CORE_INFO("TCPClient::connect[{}] - connecting to {}", "", address.toString());
    connect_ = true;
    connector->start();
}

void TCPClient::stop()
{
    connector->stop();
}

void TCPClient::newConnection(base::FileDescriptor &&fd, FdEventWatcherPtr watcher)
{
    loop->assertInLoopThread();
    TCPConnectionPtr conn(
        std::make_shared<TCPConnection>(loop, watcher, InetAddress::copyFromNative(address.sockaddr(), address.sockaddrLength())
            // , "", fd, localAddr, peerAddr
            ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    {
        std::lock_guard lock(mutex);
        connection = conn;
    }
    conn->connectEstablished(std::move(fd));
}

void TCPClient::removeConnection(const TCPConnectionPtr &conn)
{
    loop->assertInLoopThread();
    assert(loop == conn->getLoop());
    {
        std::lock_guard lock(mutex);
        assert(connection == conn);
        connection.reset();
    }
    loop->queueInLoop([conn] { conn->connectDestroyed(); });

    // HLT_INFO("TcpClient::connect[] - Reconnecting to ");
    // connector->restart();
}

void TCPClient::disconnect()
{
    std::lock_guard lock(mutex);
    if (connection)
    {
        connection->shutdown();
    }
}

} // namespace hulatang::io