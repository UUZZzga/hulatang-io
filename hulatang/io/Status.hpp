#ifndef HULATANG_IO_STATUS_HPP
#define HULATANG_IO_STATUS_HPP

#include <atomic>
namespace hulatang::io::status {

enum class EnumEventLoopStatus
{
    Null,
    Initializing,
    Initialized,
    Starting,
    Running,
    Stopping,
    Stopped,
};
typedef std::atomic<EnumEventLoopStatus> EventLoopStatus;
} // namespace hulatang::io

#endif // HULATANG_IO_STATUS_HPP
