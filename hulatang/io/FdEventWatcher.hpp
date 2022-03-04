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
    using WriteEventHandler = std::function<void(base::Buf &)>;
    using ReadEventHandler = std::function<void(base::Buf &)>;
    using ErrorEventHandler = std::function<void(std::error_condition &)>;

    void setReadHandler(const ReadEventHandler &readHandler_)
    {
        readHandler = readHandler_;
    }

    void setWriteHandler(const WriteEventHandler &writeHandler_)
    {
        writeHandler = writeHandler_;
    }

    void setErrorHandler(const ErrorEventHandler &errorHandler_)
    {
        errorHandler = errorHandler_;
    }

private:
    ReadEventHandler readHandler;
    WriteEventHandler writeHandler;
    ErrorEventHandler errorHandler;
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTWATCHER_HPP
