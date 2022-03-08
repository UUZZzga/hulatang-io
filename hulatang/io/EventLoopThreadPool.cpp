#include "hulatang/io/EventLoopThreadPool.hpp"
#include "hulatang/base/Log.hpp"

#include <memory>

namespace hulatang::io {
using std::chrono::operator""ms;

using status::EnumEventLoopStatus;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, std::string_view nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , numThreads_(0)
    , next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{
    while (status == EnumEventLoopStatus::Starting)
    {
        std::this_thread::sleep_for(1ms);
    }
    if (status == (EnumEventLoopStatus::Running))
    {
        stop();
    }
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    DLOG_TRACE;
    baseLoop_->assertInLoopThread();
    assert(numThreads_ > 0);

    status.store(EnumEventLoopStatus::Starting);
    loops_.emplace_back(baseLoop_);
    threads_.resize(numThreads_ - 1);
    int i = 1;
    for (auto &thread : threads_)
    {
        std::string threadName(name_ + std::to_string(i));
        thread = std::make_unique<EventLoopThread>(cb, threadName);

        auto *loop = thread->startLoop();
        loops_.emplace_back(loop);

        ++i;
    }
    status.store(EnumEventLoopStatus::Running);
}

void EventLoopThreadPool::stop()
{
    DLOG_TRACE;
    status.store(EnumEventLoopStatus::Stopping);
    for (auto &thread : threads_)
    {
        thread->stop();
    }
    status.store(EnumEventLoopStatus::Stopped);
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    next_ = (next_ + 1) % static_cast<int>(loops_.size());
    return loops_[next_];
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
    return loops_[hashCode % loops_.size()];
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    return loops_;
}

bool EventLoopThreadPool::started() const
{
    return status == EnumEventLoopStatus::Running;
}

} // namespace hulatang::io