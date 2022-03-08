#include "hulatang/io/TCPServer.hpp"
#include "hulatang/base/File.hpp"
#include <tuple>
#include <utility>

namespace hulatang::io {

TCPServer::TCPServer(EventLoop *_loop, std::string_view listenAddr, int port)
    : loop(_loop)
    , acceptor(_loop, listenAddr, port)
{
    acceptor.setNewConnectionCallback(
        [this](auto &&PH1, auto &&PH2) { newConnection(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
}

void TCPServer::start()
{
    loop->queueInLoop([this] { acceptor.listen(); });
}

void TCPServer::newConnection(base::FileDescriptor fd, FdEventWatcherPtr watcher)
{
    loop->assertInLoopThread();
    TCPConnectionPtr conn(std::make_shared<TCPConnection>(loop, watcher
        // , "", fd, localAddr, peerAddr
        ));
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback([&](const auto &conn) { removeConnection(conn); });
    auto it = map.emplace("", std::make_tuple(conn, std::move(fd)));
    conn->connectEstablished(std::get<base::FileDescriptor>(it.first->second));
}

void TCPServer::removeConnection(const TCPConnectionPtr &conn)
{
    loop->runInLoop([this, conn] {
        loop->assertInLoopThread();
        // LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
        // size_t n = connections_.erase(conn->name());
        // assert(n == 1);
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
    });
}

} // namespace hulatang::io