#ifndef HULATANG_IO_EVENTLOOPTHREAD_HPP
#define HULATANG_IO_EVENTLOOPTHREAD_HPP

#include "hulatang/base/ThreadSafetyAnalysis.hpp"

#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/Status.hpp"

#include <mutex>

namespace hulatang::io {
class EventLoopThread
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(ThreadInitCallback cb, std::string_view name);

    ~EventLoopThread();

    EventLoop *startLoop();

    void stop();

private:
    void threadFunc();

    EventLoop *loop_;
    std::mutex mutex_;
    std::jthread thread_;
    std::condition_variable cond_ GUARDED_BY(mutex_);
    std::string name_;
    ThreadInitCallback callback_;
    status::EventLoopStatus status;
};
} // namespace hulatang::io

#endif // HULATANG_IO_EVENTLOOPTHREAD_HPP
