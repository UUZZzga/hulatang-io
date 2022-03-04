#include "hulatang/io/Channel.hpp"

namespace hulatang::io {

Channel::Channel(EventLoop *loop)
    : loop(loop)
    , flags(kNoneEvent)
{}

void Channel::setCloseCallback(const DefaultCallback &closeCallback)
{
    this->closeCallback = closeCallback;
}
} // namespace hulatang::io