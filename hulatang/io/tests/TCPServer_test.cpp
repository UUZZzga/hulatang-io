#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/io/TCPServer.hpp"
#include <chrono>
#include <winsock2.h>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::TCPServer;

int main(int _argc, const char **_argv)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    Log::init();
    EventLoop loop;
    TCPServer server(&loop, "localhost", 8080);

    server.setConnectionCallback([](const auto &conn) {
        HLT_INFO("ConnectionCallback");
    });
    server.setMessageCallback([](const auto &conn, const auto &buf) -> void {
        HLT_INFO("{}", std::string_view(buf.buf, buf.len));
        conn->send(buf);
        conn->getLoop()->stop();
    });

    server.start();
    loop.run();

    WSACleanup();
    return 0;
}