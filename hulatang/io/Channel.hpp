#ifndef MYNET_CHANNEL_HPP_
#define MYNET_CHANNEL_HPP_

#include "hulatang/base/File.hpp"
#include <functional>
namespace hulatang::io {
class EventLoop;

class Channel
{
public:
    enum
    {
        kNoneEvent = 0,
        kReadEvent = 1,
        kWriteEvent = 2
    };
    using DefaultCallback = std::function<void(const std::shared_ptr<Channel> &)>;

public:
    explicit Channel(EventLoop *loop);
    virtual ~Channel() = default;

    EventLoop *getLoop()
    {
        return loop;
    }

    virtual void close() = 0;
    virtual void connectEstablished() = 0;

    [[nodiscard]] bool isConnected();
    [[nodiscard]] bool isConnectionPending();

    [[nodiscard]] bool isNoneEvent() const
    {
        return flags == kNoneEvent;
    }

    void enableReading()
    {
        flags |= kReadEvent;
        update();
    }
    void disableReading()
    {
        flags &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        flags |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        flags &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        flags = kNoneEvent;
        update();
    }
    [[nodiscard]] bool isWriting() const
    {
        return (flags & kWriteEvent) != 0;
    }
    [[nodiscard]] bool isReading() const
    {
        return (flags & kReadEvent) != 0;
    }

    void update() {}

    virtual void handleRead() = 0;

public:
    void setCloseCallback(const DefaultCallback &closeCallback);

    [[nodiscard]] const base::FileDescriptor &getFd() const
    {
        return fd;
    }

protected:
    EventLoop *loop;
    base::FileDescriptor fd;
    DefaultCallback closeCallback;
    int flags;
};
} // namespace hulatang::io

#endif // MYNET_CHANNEL_HPP_
