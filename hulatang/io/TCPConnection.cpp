#include "hulatang/io/TCPConnection.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/SocketModelFactory.hpp"
#include <functional>
#include <memory>
#include <utility>

namespace hulatang::io {
TCPConnection::TCPConnection(EventLoop *_loop, std::unique_ptr<Channel> channel, InetAddress peerAddr)
    : loop(_loop)
    , channel(std::move(channel))
    , peerAddr(std::move(peerAddr))
    , state(kConnecting)
{}

TCPConnection::~TCPConnection()
{
    HLT_CORE_TRACE("TCPConnection::~TCPConnection()");
}

void TCPConnection::send(const base::Buf &buf)
{
    auto fn = [this, buf] {
        sendBuffer.append(buf.buf, buf.len);
        if (!channel->isWriting())
        {
            channel->enableWriting();
        }
    };
    if (loop->isInLoopThread())
    {
        fn();
    }
    else
    {
        loop->queueInLoop(fn);
    }
}

void TCPConnection::shutdown()
{
    loop->queueInLoop([this] { closeCallback(shared_from_this()); });
}

void TCPConnection::forceClose() {
    loop->queueInLoop([this] { closeCallback(shared_from_this()); });
}

void TCPConnection::connectEstablished()
{
    assert(channel);
    loop->assertInLoopThread();
    assert(state == kConnecting);
    setState(kConnected);

    auto conn = shared_from_this();

    channel->setTie(conn);

    channel->setHandlerCallback([this](auto &&PH1) { model->handleEvent(std::forward<decltype(PH1)>(PH1)); });
    channel->setCloseCallback([this]() {
        closeCallback(shared_from_this());
    });
    channel->setErrorCallback(
        [](const std::error_condition &cond) { HLT_CORE_ERROR("error code: {}, message: {}", cond.value(), cond.message()); });

    model = SocketModelFactory::create(
        channel.get(), &recvBuffer, &sendBuffer, [this]() { messageCallback(shared_from_this(), &recvBuffer); },
        std::function<void(const char *, size_t)>());

    channel->enableReading();

    connectionCallback(conn);
}

void TCPConnection::connectDestroyed()
{
    loop->assertInLoopThread();
    HLT_CORE_TRACE("connectDestroyed::connectDestroyed");

    if (state == kConnected)
    {
        setState(kDisconnected);
        channel->disableAll();

        auto conn = shared_from_this();
        connectionCallback(conn);
    }
    channel->cancel();
    channel->clearTie();
}
} // namespace hulatang::io