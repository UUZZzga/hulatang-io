#include "hulatang/io/Acceptor.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/base/Socket.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"

namespace hulatang::io {
Acceptor::Acceptor(EventLoop *_loop, const InetAddress &address)
    : loop(_loop)
    , threadPool(nullptr)
{
    HLT_CORE_TRACE("address: {}", address.toString());
    acceptFd.bind(address.getSockaddr(), address.sockaddrLength());
}

Acceptor::~Acceptor()
{
    // TODO 结束监听
}

void Acceptor::listen()
{
    loop->assertInLoopThread();
    std::error_condition condition;
    acceptFd.listen(condition);
    if (condition)
    {
        HLT_CORE_ERROR("condition code: {}, message: {}", condition.value(), condition.message());
        return;
    }

    FdEventWatcherPtr watcher = std::make_shared<FdEventWatcher>(loop);
    watcher->setReadHandler([this](auto, auto) { acceptInLoop(); });

    loop->getFdEventManager().add(watcher, acceptFd);
    acceptFd.accept(newFd, condition);

    if (base::FileErrorCode::CONNECTING != condition.value())
    {
        HLT_CORE_ERROR("condition code: {}, message: {}", condition.value(), condition.message());
    }
}

void Acceptor::acceptInLoop()
{
    loop->assertInLoopThread();
    std::error_condition condition;

    base::socket::sockaddr_u sockaddr{};
    size_t addrlen = 0;

    auto fd = base::FileDescriptor(base::socket::accept(acceptFd.getFd(), &sockaddr, &addrlen));
    newFd.swap(fd);

    auto *nextLoop = threadPool->getNextLoop();
    FdEventWatcherPtr watcher = std::make_shared<FdEventWatcher>(nextLoop);
    newFd.getImpl()->watcher = watcher.get();
    watcher->setCloseHandler([nextLoop, watcher] {
        //  nextLoop->getFdEventManager().cancel(watcher, fd);
        abort();
    });
    // nextLoop->getFdEventManager().add(watcher, newFd);

    newConnectionCallback(base::FileDescriptor{std::move(newFd)}, watcher, InetAddress::copyFromNative(&sockaddr.sa, addrlen));
    acceptFd.accept(newFd, condition);
}
} // namespace hulatang::io