#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThreadPool.hpp"

#include <chrono>

using hulatang::base::Log;
using hulatang::io::EventLoop;
using hulatang::io::EventLoopThreadPool;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    EventLoopThreadPool pool(&loop, "pool-");
    pool.setThreadNum(8);
    loop.queueInLoop([&] {
        pool.start();
        for (int i = 0; i < 32; ++i)
        {
            auto f = [] { HLT_INFO("test"); };
            pool.getLoopForHash(i)->queueInLoop(f);
        }
        pool.stop();
        loop.stop();
    });
    loop.run();
    return 0;
}