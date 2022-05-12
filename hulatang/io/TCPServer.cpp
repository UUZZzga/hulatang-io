#include "hulatang/io/TCPServer.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"


#include <functional>
#include <memory>
#include <tuple>
#include <utility>

namespace hulatang::io {

TCPServer::TCPServer(EventLoop *_loop, std::string_view listenAddr, uint16_t port)
    : loop(_loop)
    , listenAddr(InetAddress::fromHostnameAndPort(std::string(listenAddr), port))
    , pool(std::make_unique<EventLoopThreadPool>(loop, "TCPServer"))
    , acceptor(_loop, this->listenAddr)
{
    acceptor.setNewConnectionCallback([this](auto &&PH1, auto &&PH2) {
        newConnection(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
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

void TCPServer::newConnection(std::unique_ptr<Channel> channel, InetAddress addr)
{
    loop->assertInLoopThread();
    assert(channel->getFd().getFd());
    HLT_CORE_DEBUG("TcpServer::newConnection [{}] - connection ", addr.toString());
    auto *wloop = channel->getLoop();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(wloop, std::move(channel), std::move(addr)
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([this](const auto &conn) { removeConnection(conn); });
    auto *key = conn->getPeerAddr().getSockaddr();
    auto it = map.emplace(key, conn);
    wloop->runInLoop([conn]() mutable { conn->connectEstablished(); });
}

void TCPServer::removeConnection(const TCPConnectionPtr &conn)
{
    loop->runInLoop([this, conn] {
        loop->assertInLoopThread();
        DLOG_TRACE;
        const auto &addr = conn->getPeerAddr();
        HLT_CORE_DEBUG("TcpServer::removeConnectionInLoop [{}] - connection", addr.toString());
        size_t n = map.erase(addr.getSockaddr());
        assert(n == 1);
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->runInLoop([conn] { conn->connectDestroyed(); });
    });
}

} // namespace hulatang::io