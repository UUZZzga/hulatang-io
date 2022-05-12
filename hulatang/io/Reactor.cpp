#include "hulatang/io/Reactor.hpp"
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>

static_assert(EPOLLIN == POLLIN);
static_assert(EPOLLOUT == EPOLLOUT);

namespace hulatang::io {
void Reactor::handleEvent(Channel *channel)
{
    if ((channel->revent & POLLHUP) != 0 && (channel->revent & POLLIN) == 0)
    {
        channel->handleClose();
    }
    else if ((channel->revent & (POLLERR | POLLNVAL)) != 0)
    {
        int error = 0;
        socklen_t errorsize = sizeof(error);
        getsockopt(channel->getFd().getFd(), SOL_SOCKET, SO_ERROR, &error, &errorsize);
        std::error_condition condition = std::error_condition(error, std::generic_category());
        channel->handleError(condition);
    }
    else if (channel->revent & (POLLIN | POLLPRI | POLLRDHUP))
    {
        handleRead(channel);
    }
    if (channel->revent & POLLOUT)
    {
        handleWrite(channel);
    }
}
} // namespace hulatang::io