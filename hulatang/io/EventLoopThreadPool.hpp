#ifndef HULATANG_IO_EVENTLOOPTHREADPOOL_HPP
#define HULATANG_IO_EVENTLOOPTHREADPOOL_HPP

#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/EventLoopThread.hpp"
#include <string>

namespace hulatang::io {
class EventLoopThreadPool
{
public:
    using ThreadInitCallback = std::function<void (EventLoop *)>;

    EventLoopThreadPool(EventLoop *baseLoop, std::string_view nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads)
    {
        numThreads_ = numThreads;
    }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    void stop();

    EventLoop *getNextLoop();
    EventLoop *getLoopForHash(size_t hashCode);

    std::vector<EventLoop *> getAllLoops();
    [[nodiscard]] bool started() const;

    [[nodiscard]] const std::string &name() const
    {
        return name_;
    }

private:
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
    EventLoop *baseLoop_;
    status::EventLoopStatus status;

    std::string name_;
    int numThreads_;
    int next_;
};
} // namespace hulatang::io

#endif // HULATANG_IO_EVENTLOOPTHREADPOOL_HPP
