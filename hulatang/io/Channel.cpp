#include "hulatang/io/Channel.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/EventLoop.hpp"
#include <cassert>

namespace hulatang::io {

void FreeOverList(Channel* channel);

Channel::Channel(EventLoop *loop, base::FileDescriptor &&fd)
    : loop(loop)
    , fd(std::move(fd))
    , flags(kNoneEvent)
{}

Channel::~Channel()
{
    HLT_CORE_TRACE("Channel::~Channel()");
    if(isUseOver()){
        FreeOverList(this);
    }
}

void Channel::update(int oldflag)
{
    loop->getFdEventManager().update(this);
}

void Channel::cancel()
{
    assert(isNoneEvent());
    loop->getFdEventManager().cancel(this);
}

void Channel::setCloseCallback(const DefaultCallback &closeCallback)
{
    this->closeCallback = closeCallback;
}
} // namespace hulatang::io