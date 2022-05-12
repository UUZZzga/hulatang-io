#ifndef HULATANG_IO_PROACTOR_WSACCEPTPROACTOR_HPP
#define HULATANG_IO_PROACTOR_WSACCEPTPROACTOR_HPP

#include "hulatang/base/Socket.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/io/Proactor.hpp"
#include <functional>

namespace hulatang::io {
// Windows Socket IOCP
class WSAcceptProactor : public Proactor
{
public:
    using NewConnectionCallback = std::function<void(base::socket::fd_t, InetAddress)>;

    WSAcceptProactor(Channel *channel, NewConnectionCallback &&newConnectionCallback);

    void handleRead(Channel *channel, Over *over) override;
    void handleWrite(Channel *channel, Over *over) override;

    void postAccept(Channel *channel);

private:
    void postAccept(Channel *channel, Over *over);

    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_PROACTOR_WSACCEPTPROACTOR_HPP
