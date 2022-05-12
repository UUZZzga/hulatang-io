#ifndef HULATANG_IO_REACTOR_UNIXACCEPTREACTOR_HPP
#define HULATANG_IO_REACTOR_UNIXACCEPTREACTOR_HPP

#include "hulatang/base/Socket.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/io/Reactor.hpp"

namespace hulatang::io {
class UnixAcceptReactor : public Reactor
{
public:
    using NewConnectionCallback = std::function<void(base::socket::fd_t, InetAddress)>;

    explicit UnixAcceptReactor(NewConnectionCallback &&newConnectionCallback)
        : newConnectionCallback(std::move(newConnectionCallback))
    {
    }

    void handleRead(Channel *channel) override;
    void handleWrite(Channel *channel) override;

private:
    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_REACTOR_UNIXACCEPTREACTOR_HPP
