#include "hulatang/io/Channel.hpp"

namespace hulatang::io {

Channel::Channel(EventLoop *loop, base::FileDescriptor &fd)
    : loop(loop)
    , fd(fd)
    , buffer(new char[bufferSize])
    , flags(kNoneEvent)
{}

void Channel::setCloseCallback(const DefaultCallback &closeCallback)
{
    this->closeCallback = closeCallback;
}
} // namespace hulatang::io