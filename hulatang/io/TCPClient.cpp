#include "hulatang/io/TCPClient.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/SocketModelFactory.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include <memory>

namespace hulatang::io {

TCPClient::TCPClient(EventLoop *_loop, InetAddress _address)
    : loop(_loop)
    , address(std::move(_address))
    , connector(std::make_shared<Connector>(_loop, address))
{
    connector->setNewConnectionCallback(
        [this](std::unique_ptr<Channel> channel) { newConnection(std::move(channel)); });
}

void TCPClient::connect()
{
    HLT_CORE_DEBUG("TCPClient::connect[{}] - connecting to {}", "", address.toString());
    connect_ = true;
    connector->start();
}

void TCPClient::stop()
{
    connector->stop();
}

void TCPClient::newConnection(std::unique_ptr<Channel> channel)
{
    loop->assertInLoopThread();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(
        loop, std::move(channel), InetAddress::copyFromNative(address.getSockaddr(), address.sockaddrLength())
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    {
        std::lock_guard lock(mutex);
        connection = conn;
    }
    conn->connectEstablished();
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