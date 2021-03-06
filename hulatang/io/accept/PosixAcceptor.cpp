#include "hulatang/io/Acceptor.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/base/Socket.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"
#include "hulatang/io/SocketModelFactory.hpp"
#include "hulatang/io/reactor/UnixAcceptReactor.hpp"
#include <memory>
#include <system_error>

namespace hulatang::io {
Acceptor::Acceptor(EventLoop *_loop, const InetAddress &address)
    : loop(_loop)
    , threadPool(nullptr)
    , idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    acceptFd.socket(address.getSockaddr(), address.sockaddrLength());
    acceptFd.bind(address.getSockaddr(), address.sockaddrLength());
    acceptChannel = std::make_unique<Channel>(loop, std::move(acceptFd));

    acceptChannel->setErrorCallback([this](std::error_condition condition) mutable {
        if (condition == std::errc::too_many_files_open)
        {
            HLT_CORE_WARN("too many files open");
            idleFd.close();

            auto fd = ::accept(acceptChannel->getFd().getFd(), nullptr, nullptr);
            if (fd >= 0)
            {
                ::close(fd);
            }

            base::FileDescriptor idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC));
            idleFd.swap(idleFd_);
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

    model = std::make_unique<UnixAcceptReactor>([this](base::socket::fd_t fd, InetAddress addr) { acceptInLoop(fd, std::move(addr)); });
    acceptChannel->setHandlerCallback([this](Channel *channel) { model->handleEvent(channel); });

    std::error_condition condition;
    acceptChannel->getFd().listen(condition);
    if (condition)
    {
        HLT_CORE_ERROR("condition code: {}, message: {}", condition.value(), condition.message());
        return;
    }

    loop->getFdEventManager().add(acceptChannel.get());
    acceptChannel->enableReading();
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