#ifndef HULATANG_IO_TCPSERVER_HPP
#define HULATANG_IO_TCPSERVER_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/base/File.hpp"
#include "hulatang/io/Acceptor.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/io/InetAddress.hpp"

#include <functional>
#include <memory>
#include <string_view>
#include <tuple>
#include <unordered_map>
namespace hulatang::io {

class TCPServer
{
public:
    using ConnectionCallback = std::function<void(const TCPConnectionPtr &)>;
    using MessageCallback = std::function<void(const TCPConnectionPtr &, base::Buffer *)>;

    TCPServer(EventLoop *_loop, std::string_view listenAddr, uint16_t port);

    void setThreadNum(int numThreads);

    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback = std::move(cb);
    }

    void setMessageCallback(MessageCallback cb)
    {
        messageCallback = std::move(cb);
    }

    void start();

private:
    void newConnection(std::unique_ptr<Channel> channel, InetAddress addr);

    void removeConnection(const TCPConnectionPtr &conn);

private:
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    EventLoop *loop;
    InetAddress listenAddr;
    std::unique_ptr<EventLoopThreadPool> pool;
    std::unordered_map<struct sockaddr *, TCPConnectionPtr> map;
    Acceptor acceptor;
};
} // namespace hulatang::io
#endif // HULATANG_IO_TCPSERVER_HPP
