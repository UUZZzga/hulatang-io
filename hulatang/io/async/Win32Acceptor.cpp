#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/io/Acceptor.hpp"

#include "hulatang/io/EventLoop.hpp"

namespace hulatang::io {
Acceptor::Acceptor(EventLoop *_loop, std::string_view listenAddr, int port)
    : loop(_loop)
{
    acceptFd.bind(listenAddr, port);
}

Acceptor::~Acceptor() {}

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
    FdEventWatcherPtr watcher = std::make_shared<FdEventWatcher>(loop);
    loop->getFdEventManager().add(watcher, newFd);
    newConnectionCallback(base::FileDescriptor{std::move(newFd)}, watcher);
    acceptFd.accept(newFd, condition);
}
} // namespace hulatang::io