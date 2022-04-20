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
    TCPClient client(&loop, InetAddress::fromHostnameAndService("localhost", "http"));

    client.setConnectionCallback([](const auto &conn) { HLT_INFO("ConnectionCallback"); });
    client.setMessageCallback([](const auto &conn, const auto &buf) -> void {
        HLT_INFO("{}", std::string_view(buf.buf, buf.len));
        conn->send(buf);
        conn->getLoop()->stop();
    });

    loop.queueInLoop([&client] { client.connect(); });
    loop.run();

    return 0;
}