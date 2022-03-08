#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThread.hpp"

#include <chrono>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::EventLoopThread;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    EventLoopThread thread(
        [&](EventLoop *newLoop) {
            newLoop->queueInLoop([=, &loop] {
                newLoop->stop();
                loop.stop();
            });
        },
        "test thread");
    loop.queueInLoop([&] { thread.startLoop(); });
    loop.run();
    return 0;
}