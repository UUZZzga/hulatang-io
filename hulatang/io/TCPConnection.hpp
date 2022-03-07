#ifndef HULATANG_IO_TCPCONNECTION_HPP
#define HULATANG_IO_TCPCONNECTION_HPP

#include "hulatang/base/Buf.hpp"
#include "hulatang/io/SocketChannel.hpp"

#include <atomic>
#include <memory>

namespace hulatang::io {
class TCPConnection;
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;
class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    using SocketChannelPtr = std::shared_ptr<SocketChannel>;
    using ConnectionCallback = std::function<void(const TCPConnectionPtr &)>;
    using MessageCallback = std::function<void(const TCPConnectionPtr &, const base::Buf &)>;
    using CloseCallback = std::function<void(const TCPConnectionPtr &)>;

    TCPConnection(EventLoop *_loop, const FdEventWatcherPtr &watcher);

    void send(const base::Buf &buf);

    void shutdown(); // NOT thread safe, no simultaneous calling
    void forceClose();

    void startRead();
    void stopRead();

    // called when TcpServer accepts a new connection
    void connectEstablished(base::FileDescriptor &fd); // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed(); // should be called only once

    void setConnectionCallback(const ConnectionCallback &connectionCallback_)
    {
        connectionCallback = connectionCallback_;
    }

    void setMessageCallback(const MessageCallback &messageCallback_)
    {
        messageCallback = messageCallback_;
    }

    void setCloseCallback(const CloseCallback &closeCallback_)
    {
        closeCallback = closeCallback_;
    }

    [[nodiscard]] EventLoop *getLoop() const
    {
        return loop;
    }

private:
    EventLoop *loop;
    SocketChannelPtr channel;
    FdEventWatcherPtr::weak_type watcherWPtr;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    CloseCallback closeCallback;
    std::atomic_int32_t sending;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TCPCONNECTION_HPP
