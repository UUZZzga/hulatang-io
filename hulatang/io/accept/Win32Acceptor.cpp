#include "hulatang/io/Acceptor.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"
#include "hulatang/io/proactor/WSAcceptProactor.hpp"

namespace hulatang::io {
Acceptor::Acceptor(EventLoop *_loop, const InetAddress &address)
    : loop(_loop)
    , threadPool(nullptr)
{
    acceptFd.socket(address.getSockaddr(), address.sockaddrLength());
    acceptFd.bind(address.getSockaddr(), address.sockaddrLength());
    acceptChannel = std::make_unique<Channel>(loop, std::move(acceptFd));

    acceptChannel->setErrorCallback([](std::error_condition condition) mutable {
        if (condition == std::errc::too_many_files_open)
        {
            HLT_CORE_WARN("too many files open");
        }
        else
        {
            HLT_CORE_ERROR("acceptChannel - error code: {}, message: {}", condition.value(), condition.message());
        }
    });
}

Acceptor::~Acceptor()
{
    // TODO 结束监听
}

void Acceptor::listen()
{
    loop->assertInLoopThread();
    model = std::make_unique<WSAcceptProactor>(acceptChannel.get(), [this](base::socket::fd_t fd, InetAddress addr) { acceptInLoop(fd, std::move(addr)); });
    acceptChannel->setHandlerCallback([this](Channel *channel) { model->handleEvent(channel); });

    std::error_condition condition;
    acceptChannel->getFd().listen(condition);
    if (condition)
    {
        HLT_CORE_ERROR("condition code: {}, message: {}", condition.value(), condition.message());
        return;
    }

    loop->getFdEventManager().add(acceptChannel.get());

    static_cast<WSAcceptProactor *>(model.get())->postAccept(acceptChannel.get());
}

void Acceptor::acceptInLoop(base::socket::fd_t fd, InetAddress addr)
{
    loop->assertInLoopThread();

    auto *nextLoop = threadPool->getNextLoop();

    auto channel = std::make_unique<Channel>(nextLoop, base::FileDescriptor{fd});

    nextLoop->runInLoop([nextLoop, channelPtr{channel.get()}]() { nextLoop->getFdEventManager().add(channelPtr); });
    newConnectionCallback(std::move(channel), std::move(addr));
}
} // namespace hulatang::io