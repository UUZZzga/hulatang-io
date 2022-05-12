#include "hulatang/io/reactor/UnixAcceptReactor.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/Socket.hpp"
#include "hulatang/io/reactor/UnixReactor.hpp"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <sys/uio.h>
#include <unistd.h>

namespace hulatang::io {

void UnixAcceptReactor::handleRead(Channel *channel)
{
    base::socket::sockaddr_u sockaddr{};
    size_t addrlen = 0;

    auto sockfd = base::socket::accept(channel->getFd().getFd(), &sockaddr, &addrlen);
    if (sockfd >= 0)
    {
        newConnectionCallback(sockfd, InetAddress::copyFromNative(&sockaddr.sa, addrlen));
    }
    else
    {
        std::error_condition condition = std::error_condition(errno, std::generic_category());
        channel->handleError(condition);
    }
}

void UnixAcceptReactor::handleWrite(Channel *channel)
{
    abort();
}
} // namespace hulatang::io
