#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/io/TCPServer.hpp"
#include <chrono>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::TCPServer;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    TCPServer server(&loop, "0.0.0.0", 8080);

    server.setThreadNum(1);

    server.setConnectionCallback([](const auto &conn) {
        if (conn->isConnected())
        {
            HLT_INFO("new conn - {}", conn->getPeerAddr().toString());
        }
        else
        {
            HLT_INFO("des conn - {}", conn->getPeerAddr().toString());
        }
    });
    server.setMessageCallback([](const auto &conn, auto *buf) -> void {
        std::string sbuf{buf->retrieveAllAsString()};
        HLT_INFO("message {} - {}", conn->getPeerAddr().toString(), sbuf);
        conn->getLoop()->runInLoop([&] { conn->send({sbuf.data(), sbuf.size()}); });
    });

    server.start();
    loop.run();
    return 0;
}