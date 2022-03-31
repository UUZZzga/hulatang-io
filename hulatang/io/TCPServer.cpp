#include "hulatang/io/TCPServer.hpp"
#include "hulatang/base/File.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <utility>

namespace hulatang::io {

TCPServer::TCPServer(EventLoop *_loop, std::string_view listenAddr, uint16_t port)
    : loop(_loop)
    , pool(std::make_unique<EventLoopThreadPool>(loop, "TCPServer"))
    , listenAddr(InetAddress::fromHostnameAndPort(std::string(listenAddr), port))
    , acceptor(_loop, this->listenAddr)
{
    acceptor.setNewConnectionCallback([this](auto &&PH1, auto &&PH2, auto &&PH3) {
        newConnection(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3));
    });
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

void TCPServer::newConnection(base::FileDescriptor fd, FdEventWatcherPtr watcher, InetAddress addr)
{
    loop->assertInLoopThread();
    assert(fd.getFd());
    HLT_CORE_INFO("TcpServer::newConnection [{}] - connection ", addr.toString());
    auto *wloop = watcher->getLoop();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(wloop, watcher, std::move(addr)
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    auto key = conn->getPeerAddr().toString();
    auto it = map.emplace(key, conn);
    auto fdPtr = std::make_shared<base::FileDescriptor>(std::move(fd));
    wloop->runInLoop([conn, fd{fdPtr}]() mutable { conn->connectEstablished(std::move(*fd)); });
}

void TCPServer::removeConnection(const TCPConnectionPtr &conn)
{
    loop->runInLoop([this, conn] {
        loop->assertInLoopThread();
        auto addr = conn->getPeerAddr().toString();
        HLT_CORE_INFO("TcpServer::removeConnectionInLoop [{}] - connection", addr);
        size_t n = map.erase(addr);
        assert(n == 1);
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
    });
}

} // namespace hulatang::io