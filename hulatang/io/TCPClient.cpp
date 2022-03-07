#include "hulatang/io/TCPClient.hpp"

#include "hulatang/io/TcpConnection.hpp"
#include <memory>

namespace hulatang::io {

TCPClient::TCPClient(EventLoop *_loop, std::string_view _host, int _port)
    : loop(_loop)
    , connector(std::make_shared<Connector>(_loop, _host, _port))
    , host(_host)
    , port(_port)
{
    connector->setNewConnectionCallback([&](base::FileDescriptor &fd, FdEventWatcherPtr watcher) { newConnection(fd, watcher); });
}

void TCPClient::connect()
{
    HLT_CORE_INFO("TCPClient::connect[{}] - connecting to {}:{}", "", host, port);
    connect_ = true;
    connector->start();
}

void TCPClient::stop()
{
    connector->stop();
}

void TCPClient::newConnection(base::FileDescriptor &fd, FdEventWatcherPtr watcher)
{
    loop->assertInLoopThread();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(loop, watcher
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    {
        std::lock_guard lock(mutex);
        connection = conn;
    }
    conn->connectEstablished(fd);
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