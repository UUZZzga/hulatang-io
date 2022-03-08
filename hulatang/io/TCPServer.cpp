#include "hulatang/io/TCPServer.hpp"
#include "hulatang/base/File.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"

#include <memory>
#include <tuple>
#include <utility>

namespace hulatang::io {

TCPServer::TCPServer(EventLoop *_loop, std::string_view listenAddr, int port)
    : loop(_loop)
    , pool(std::make_unique<EventLoopThreadPool>(loop, "TCPServer"))
    , acceptor(_loop, listenAddr, port)
{
    acceptor.setNewConnectionCallback(
        [this](auto &&PH1, auto &&PH2) { newConnection(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
}

void TCPServer::setThreadNum(int numThreads)
{
    pool->setThreadNum(numThreads);
}

void TCPServer::start()
{
    loop->queueInLoop([this] {
        pool->start();
        acceptor.setThreadPool(pool.get());
        acceptor.listen();
    });
}

void TCPServer::newConnection(base::FileDescriptor fd, FdEventWatcherPtr watcher)
{
    loop->assertInLoopThread();
    auto wloop = watcher->getLoop();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(wloop, watcher
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    auto key = "" + std::to_string(fd.getFd()); // TODO conn name
    auto it = map.emplace(key, std::make_tuple(conn, std::move(fd)));
    wloop->runInLoop([conn, fd = &std::get<base::FileDescriptor>(it.first->second)] { conn->connectEstablished(*fd); });
}

void TCPServer::removeConnection(const TCPConnectionPtr &conn)
{
    loop->runInLoop([this, conn] {
        loop->assertInLoopThread();
        // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
        // size_t n = connections_.erase(conn->name()); // TODO conn name
        // assert(n == 1);
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
    });
}

} // namespace hulatang::io