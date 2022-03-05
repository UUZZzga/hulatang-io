#ifndef HULATANG_IO_FDEVENTWATCHER_HPP
#define HULATANG_IO_FDEVENTWATCHER_HPP

#include "hulatang/base/Buf.hpp"
#include "hulatang/io/EventWatcher.hpp"
#include <system_error>

namespace hulatang::io {
class FdEventWatcher;

using FdEventWatcherPtr = std::shared_ptr<FdEventWatcher>;

class FdEventWatcher : public ClosableEventWatcher
{
public:
    using WriteEventHandler = std::function<void(size_t)>;
    using ReadEventHandler = std::function<void(size_t)>;
    using ErrorEventHandler = std::function<void(std::error_condition &)>;

    void setReadHandler(const ReadEventHandler &readHandler_)
    {
        readHandler = readHandler_;
    }

    WriteEventHandler getWriteHandler() const
    {
        return writeHandler;
    }
    void setWriteHandler(const WriteEventHandler &writeHandler_)
    {
        writeHandler = writeHandler_;
    }

    void setErrorHandler(const ErrorEventHandler &errorHandler_)
    {
        errorHandler = errorHandler_;
    }

    void setCloseHandler(const EventHandler &closeHandler_)
    {
        closeHandler = closeHandler_;
    }

    void readHandle(size_t num)
    {
        readHandler(num);
    }

    void writeHandle(size_t num)
    {
        writeHandler(num);
    }

    void openHandle()
    {
        openHandler();
    }

    void closeHandle()
    {
        closeHandler();
    }

private:
    ReadEventHandler readHandler;
    WriteEventHandler writeHandler;
    ErrorEventHandler errorHandler;
    EventHandler openHandler;
    EventHandler closeHandler;
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTWATCHER_HPP
