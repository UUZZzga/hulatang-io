#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include <chrono>

using hulatang::base::Log;
using hulatang::io::EventLoop;

using std::chrono::operator""s;
using std::chrono::operator""ms;

int main(int _argc, const char **_argv)
{
    Log::init();
    EventLoop loop;
    loop.queueInLoop([&] {
        HLT_INFO("OK");
        loop.runEvery(200ms, [] { HLT_INFO("runEvery 200ms"); });
        loop.runAfter(1s, [] { HLT_INFO("runAfter 1s"); });
        loop.runAfter(1500ms, [] { HLT_INFO("runAfter 1.5s"); });
        loop.runAfter(2000ms, [&loop] { loop.stop(); });
    });
    loop.run();
    return 0;
}