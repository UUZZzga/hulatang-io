#ifndef HULATANG_IO_TCPCONNECTION_HPP
#define HULATANG_IO_TCPCONNECTION_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/Model.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include <any>
#include <array>
#include <atomic>
#include <memory>

namespace hulatang::io {
class TCPConnection;
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;
class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    using ConnectionCallback = std::function<void(const TCPConnectionPtr &)>;
    using MessageCallback = std::function<void(const TCPConnectionPtr &, base::Buffer *)>;
    using CloseCallback = std::function<void(const TCPConnectionPtr &)>;
    using Context = std::any;
    // msvc: sizeof(Context) == 64
    static constexpr size_t ContextSize = 4;

    enum StateE
    {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };

    TCPConnection(EventLoop *_loop, std::unique_ptr<Channel> channel, InetAddress peerAddr);
    ~TCPConnection();

    void send(const base::Buf &buf);

    void shutdown(); // NOT thread safe, no simultaneous calling
    void forceClose();

    void startRead();
    void stopRead();

    // called when TcpServer accepts a new connection
    void connectEstablished(); // should be called only once
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

    void setState(StateE state)
    {
        this->state = state;
    }

    bool isConnecting()
    {
        return state == kConnecting;
    }

    bool isConnected()
    {
        return state == kConnected;
    }

    const std::any &context(size_t index) const
    {
        return contexts.at(index);
    }

    std::any &context(size_t index)
    {
        return contexts.at(index);
    }

    void setContext(size_t index, std::any &&value)
    {
        contexts.at(index) = std::move(value);
    }

    const InetAddress &getPeerAddr() const
    {
        return peerAddr;
    }

private:
    EventLoop *loop;
    std::unique_ptr<Channel> channel;
    std::unique_ptr<Model> model;
    base::Buffer sendBuffer;
    base::Buffer recvBuffer;
    InetAddress peerAddr;
    std::atomic_int32_t sending;
    std::atomic<StateE> state;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    CloseCallback closeCallback;
    std::array<Context, ContextSize> contexts;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TCPCONNECTION_HPP
