#ifndef HULATANG_IO_EVENTWATCHER_HPP
#define HULATANG_IO_EVENTWATCHER_HPP

#include <functional>
namespace hulatang::io {
class EventWatcher
{
public:
    typedef std::function<void()> EventHandler;

    virtual ~EventWatcher();

    void cancel();

    void setHandler(const EventHandler &_handler) { handler = _handler; }
protected:
    EventWatcher();

    virtual bool DoInit() = 0;
    virtual void DoClose() {}

private:
    EventHandler handler;
    EventHandler cancelCallback;
    bool attached;
};
} // namespace hulatang::io

#endif // HULATANG_IO_EVENTWATCHER_HPP
