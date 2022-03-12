#include "hulatang/http/HttpClient.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include <winsock2.h>

using hulatang::base::Log;
using hulatang::http::HttpClient;
using hulatang::http::URL;
using hulatang::io::EventLoop;

int main(int _argc, const char **_argv)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    Log::init();
    EventLoop loop;
    HttpClient clinet(&loop);
    clinet.get("http://localhost:8080/buiding/queryAll", [](auto &res) {
        HLT_INFO("{}", res.body());
    });

    loop.run();

    WSACleanup();
    return 0;
}