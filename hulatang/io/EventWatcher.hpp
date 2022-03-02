#ifndef HULATANG_IO_EVENTWATCHER_HPP
#define HULATANG_IO_EVENTWATCHER_HPP

#include <algorithm>
#include <functional>
namespace hulatang::io {
class EventLoop;

class EventWatcher
{
public:
    typedef std::function<void()> EventHandler;

    virtual ~EventWatcher() = default;

    void setHandler(const EventHandler &_handler)
    {
        handler = _handler;
    }

    void setHandler(EventHandler &&_handler)
    {
        handler = move(_handler);
    }

protected:
    EventWatcher() = default;

protected:
    EventHandler handler;
};

class ClosableEventWatcher : public EventWatcher
{
public:
    typedef std::function<void()> EventHandler;

    virtual ~ClosableEventWatcher();

    void cancel();

    virtual void DoClose() {}

protected:
    EventLoop *loop;
    EventHandler cancelCallback;
    bool attached;
};
} // namespace hulatang::io

#endif // HULATANG_IO_EVENTWATCHER_HPP
