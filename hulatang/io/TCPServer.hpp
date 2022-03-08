#ifndef HULATANG_IO_TCPSERVER_HPP
#define HULATANG_IO_TCPSERVER_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/Acceptor.hpp"
#include "hulatang/io/TcpConnection.hpp"
#include <functional>
#include <string_view>
#include <tuple>
#include <unordered_map>
namespace hulatang::io {

class TCPServer
{
public:
    using ConnectionCallback = std::function<void(const TCPConnectionPtr &)>;
    using MessageCallback = std::function<void(const TCPConnectionPtr &, const base::Buf &)>;

    TCPServer(EventLoop *_loop, std::string_view listenAddr, int port);

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
    void newConnection(base::FileDescriptor fd, FdEventWatcherPtr watcher);

    void removeConnection(const TCPConnectionPtr &conn);

private:
    EventLoop *loop;
    Acceptor acceptor;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    std::unordered_map<std::string, std::tuple<TCPConnectionPtr, base::FileDescriptor>> map;
};
} // namespace hulatang::io
#endif // HULATANG_IO_TCPSERVER_HPP
