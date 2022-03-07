#ifndef HULATANG_IO_TCPCLIENT_HPP
#define HULATANG_IO_TCPCLIENT_HPP

#include "hulatang/base/Buf.hpp"
#include "hulatang/io/Connector.hpp"
#include "hulatang/io/TcpConnection.hpp"
#include <memory>
#include <mutex>
namespace hulatang::io {
class TCPClient
{
public:
    using ConnectionCallback = std::function<void(const TCPConnectionPtr &)>;
    using MessageCallback = std::function<void(const TCPConnectionPtr &,const base::Buf&)>;

    TCPClient(EventLoop *_loop, std::string_view _host, int _port);

    void connect();
    void disconnect();
    void stop();

    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback = std::move(cb);
    }

    void setMessageCallback(MessageCallback cb)
    {
        messageCallback = std::move(cb);
    }

private:
    void newConnection(base::FileDescriptor &fd, FdEventWatcherPtr watcher);
    void removeConnection(const TCPConnectionPtr &conn);

private:
    EventLoop *loop;
    std::shared_ptr<Connector> connector;
    TCPConnectionPtr connection;
    std::mutex mutex;
    std::string host;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    int port;
    bool connect_;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TCPCLIENT_HPP
