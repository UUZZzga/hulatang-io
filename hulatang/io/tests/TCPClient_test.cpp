#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPClient.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include <chrono>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::InetAddress;
using hulatang::io::TCPClient;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    TCPClient client(&loop, InetAddress::fromHostnameAndPort("localhost", 8080));

    client.setConnectionCallback([](const auto &conn) { HLT_INFO("ConnectionCallback"); });
    client.setMessageCallback([](const auto &conn, auto *buf) -> void {
        std::string sbuf{buf->retrieveAllAsString()};
        HLT_INFO("{}", sbuf);
        conn->getLoop()->runInLoop([&] { conn->send({sbuf.data(), sbuf.size()}); });
        conn->getLoop()->stop();
    });

    loop.queueInLoop([&client] { client.connect(); });
    loop.run();

    return 0;
}