#include "hulatang/io/Channel.hpp"

namespace hulatang::io {

Channel::Channel(EventLoop *loop, base::FileDescriptor &&fd)
    : loop(loop)
    , fd(std::move(fd))
// #if HLT_PLATFORM_WINDOWS
    , buffer(new char[bufferSize])
// #endif
    , flags(kNoneEvent)
{}

void Channel::setCloseCallback(const DefaultCallback &closeCallback)
{
    this->closeCallback = closeCallback;
}
} // namespace hulatang::io