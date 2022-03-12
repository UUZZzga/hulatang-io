#include "hulatang/io/EventLoopThread.hpp"
#include "hulatang/io/Status.hpp"
#include <mutex>
#include <thread>
#include <utility>

namespace hulatang::io {
using status::EnumEventLoopStatus;
using std::chrono::operator""ms;

EventLoopThread::EventLoopThread(ThreadInitCallback cb, std::string_view name)
    : loop_(nullptr)
    , name_(name)
    , callback_(std::move(cb))
{
    status.store(EnumEventLoopStatus::Stopped);
}

EventLoopThread::~EventLoopThread()
{
    if (status != EnumEventLoopStatus::Stopped)
    {
        stop();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    assert(!thread_.joinable());
    thread_ = std::jthread(&EventLoopThread::threadFunc, this);

    EventLoop *loop = nullptr;
    {
        std::unique_lock lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::stop()
{
    while (status != EnumEventLoopStatus::Running)
    {
        std::this_thread::sleep_for(1ms);
    }
    status.store(EnumEventLoopStatus::Stopping);

    loop_->stop();
    thread_.join();
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    status.store(EnumEventLoopStatus::Starting);

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::lock_guard lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    status.store(EnumEventLoopStatus::Running);
    loop.run();
    std::lock_guard lock(mutex_);
    loop_ = nullptr;
    status.store(EnumEventLoopStatus::Stopped);
}

} // namespace hulatang::io