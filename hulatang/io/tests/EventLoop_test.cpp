#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"

using hulatang::base::Log;
using hulatang::io::EventLoop;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    loop.queueInLoop([]{
        HLT_INFO("OK");
    });
    loop.run();
    return 0;
}