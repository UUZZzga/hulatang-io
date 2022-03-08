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
    using UpdateCallback = std::function<void(int)>;
    using DefaultCallback = std::function<void(const std::shared_ptr<Channel> &)>;

public:
    explicit Channel(EventLoop *loop, base::FileDescriptor &fd);
    virtual ~Channel() = default;

    EventLoop *getLoop()
    {
        return loop;
    }

    virtual void close() = 0;

    [[nodiscard]] bool isNoneEvent() const
    {
        return flags == kNoneEvent;
    }

    void enableReading()
    {
        int old = flags;
        flags |= kReadEvent;
        update(old);
    }
    void disableReading()
    {
        int old = flags;
        flags &= ~kReadEvent;
        update(old);
    }
    void enableWriting()
    {
        int old = flags;
        flags |= kWriteEvent;
        update(old);
    }
    void disableWriting()
    {
        int old = flags;
        flags &= ~kWriteEvent;
        update(old);
    }
    void disableAll()
    {
        int old = flags;
        flags = kNoneEvent;
        update(old);
    }
    [[nodiscard]] bool isWriting() const
    {
        return (flags & kWriteEvent) != 0;
    }
    [[nodiscard]] bool isReading() const
    {
        return (flags & kReadEvent) != 0;
    }

    virtual void update(int oldflag) = 0;

    virtual void handleRead() = 0;

public:
    void setCloseCallback(const DefaultCallback &closeCallback);

    [[nodiscard]] base::FileDescriptor &getFd()
    {
        return fd;
    }

protected:
    EventLoop *loop;
    base::FileDescriptor &fd;
    DefaultCallback closeCallback;
#if _WIN32
    static constexpr size_t bufferSize = 4096;
    std::unique_ptr<char[]> buffer;
#endif
    int flags;
};
} // namespace hulatang::io

#endif // MYNET_CHANNEL_HPP_
