#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPClient.hpp"
#include <chrono>
#include <winsock2.h>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::TCPClient;

int main(int _argc, const char **_argv)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    Log::init();
    EventLoop loop;
    TCPClient client(&loop, "localhost", 8080);

    client.setConnectionCallback([](const auto &conn) { HLT_INFO("ConnectionCallback"); });
    client.setMessageCallback([](const auto &conn) {

    });

    loop.queueInLoop([&] { client.connect(); });
    loop.run();

    WSACleanup();
    return 0;
}