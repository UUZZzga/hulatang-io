#include "hulatang/io/reactor/UnixConnectReactor.hpp"

#include "hulatang/io/reactor/UnixReactor.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include <cstdlib>
#include <sys/uio.h>
#include <unistd.h>

namespace hulatang::io {
std::unique_ptr<Model> SocketModelFactory::createConnect(Channel *channel, , sockaddr *addr, size_t addrLen)
{
    return std::make_unique<UnixConnectReactor>();
}

void UnixConnectReactor::handleRead(Channel *channel)
{
    abort();
}

void UnixConnectReactor::handleWrite(Channel *channel)
{
    int error = 0;
    socklen_t errorsize = sizeof(error);
    getsockopt(channel->getFd().getFd(), SOL_SOCKET, SO_ERROR, &error, &errorsize);

    // 判断连接成功
    if (error == 0)
    {
        channel->handleWrite();
        // 连接成功ACK分节可能和数据一起发送来
        // 所以可以执行后面的逻辑
    }
    else
    {
        std::error_condition ec;
        // TODO: error code
        channel->handleError(ec);
        // 跳过读取
    }
}
} // namespace hulatang::io
