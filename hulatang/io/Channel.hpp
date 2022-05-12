#ifndef HULATANG_IO_CHANNEL_HPP
#define HULATANG_IO_CHANNEL_HPP

#include "hulatang/base/File.hpp"
#include <cstdint>
#include <functional>
#include <system_error>

namespace hulatang::io {
class EventLoop;

class Channel
{
public:
    enum
    {
        kNoneEvent = 0,
        kReadEvent = 1,
        kWriteEvent = 2,
        kUseOver = 4,
    };
    using UpdateCallback = std::function<void(int)>;
    using HandlerCallback = std::function<void(Channel *)>;
    using DefaultCallback = std::function<void()>;
    using ErrorCallback = std::function<void(const std::error_condition &)>;

    friend class Proactor;
    friend class Reactor;

public:
    explicit Channel(EventLoop *loop, base::FileDescriptor &&fd);
    ~Channel();

    EventLoop *getLoop()
    {
        return loop;
    }

    [[nodiscard]] bool isNoneEvent() const
    {
        return flags == kNoneEvent;
    }

    void enableOver()
    {
        flags |= kUseOver;
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
    [[nodiscard]] bool isUseOver() const
    {
        return (flags & kUseOver) != 0;
    }

    void update(int oldflag);

    void cancel();

    void handleRead()
    {
        readCallback();
    }
    void handleWrite()
    {
        writeCallback();
    }
    void handleClose()
    {
        closeCallback();
    }

    void setCloseCallback(const DefaultCallback &closeCallback);

    [[nodiscard]] const base::FileDescriptor &getFd() const
    {
        return fd;
    }

    [[nodiscard]] base::FileDescriptor &getFd()
    {
        return fd;
    }

    void handler()
    {
        handlerCallback(this);
    }

    void setHandlerCallback(const HandlerCallback &handlerCallback_)
    {
        handlerCallback = handlerCallback_;
    }

    void handleError(const std::error_condition &ec)
    {
        errorCallback(ec);
    }
    void setErrorCallback(const ErrorCallback &errorCallback_)
    {
        errorCallback = errorCallback_;
    }

    void setReadCallback(const DefaultCallback &readCallback_)
    {
        readCallback = readCallback_;
    }

    void setWriteCallback(const DefaultCallback &writeCallback_)
    {
        writeCallback = writeCallback_;
    }

    uint32_t getREvent() const
    {
        return revent;
    }

    void setREvent(uint32_t revent_)
    {
        revent = revent_;
    }

    void *getOver() const
    {
        return over;
    }
    void setOver(void *over_)
    {
        over = over_;
    }

    void *getROver() const
    {
        return rover;
    }
    void setROver(void *rover_)
    {
        rover = rover_;
    }

    std::shared_ptr<void> getTie() const { return tie; }
    void setTie(const std::shared_ptr<void> &tie_) { tie = tie_; }
    void clearTie() { tie.reset(); }

protected:
    EventLoop *loop;
    base::FileDescriptor fd;
    std::shared_ptr<void> tie;

    DefaultCallback readCallback;
    DefaultCallback writeCallback;
    DefaultCallback closeCallback;
    HandlerCallback handlerCallback;
    ErrorCallback errorCallback;

    union
    {
        void *over{};
        uint32_t event;
    };

    union
    {
        void *rover{};
        uint32_t revent;
    };

    int flags;
};
} // namespace hulatang::io

#endif // HULATANG_IO_CHANNEL_HPP
