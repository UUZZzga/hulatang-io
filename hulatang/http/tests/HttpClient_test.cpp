#include "hulatang/http/HttpClient.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"

using hulatang::base::Log;
using hulatang::http::HttpClient;
using hulatang::http::URL;
using hulatang::io::EventLoop;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    HttpClient clinet(&loop);
    clinet.get("http://www.baidu.com", [](auto &res) { HLT_INFO("{}", res.body()); });

    loop.run();
    return 0;
}