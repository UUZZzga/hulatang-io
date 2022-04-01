#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPConnection.hpp"
#include "hulatang/io/TCPServer.hpp"
#include <chrono>
#if HLT_PLATFORM_WINDOWS
#    include <winsock2.h>
#endif

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::TCPServer;

int main(int _argc, const char **_argv)
{
#if HLT_PLATFORM_WINDOWS
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    Log::init();
    EventLoop loop;
    TCPServer server(&loop, "localhost", 8080);

    server.setThreadNum(4);

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
    server.setMessageCallback([](const auto &conn, const auto &buf) -> void {
        HLT_INFO("message {} - {}", conn->getPeerAddr().toString(), std::string_view(buf.buf, buf.len));
        conn->send(buf);
    });

    server.start();
    loop.run();
#if HLT_PLATFORM_WINDOWS
    WSACleanup();
#endif
    return 0;
}