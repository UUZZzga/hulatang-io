#ifndef HULATANG_IO_REACTOR_UNIXCONNECTREACTOR_HPP
#define HULATANG_IO_REACTOR_UNIXCONNECTREACTOR_HPP

#include "hulatang/io/Reactor.hpp"

namespace hulatang::io {
class UnixConnectReactor : public Reactor
{
public:
    void handleRead(Channel *channel) override;
    void handleWrite(Channel *channel) override;
};
} // namespace hulatang::io

#endif // HULATANG_IO_REACTOR_UNIXCONNECTREACTOR_HPP
