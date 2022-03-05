#ifndef HULATANG_IO_TCPCONNECTION_HPP
#define HULATANG_IO_TCPCONNECTION_HPP

#include "hulatang/io/SocketChannel.hpp"

#include <memory>

namespace hulatang::io {
class TCPConnection
{
public:
    using SocketChannelPtr = std::shared_ptr<SocketChannel>;

private:
    SocketChannelPtr channel;
};
} // namespace hulatang::io

#endif // HULATANG_IO_TCPCONNECTION_HPP
