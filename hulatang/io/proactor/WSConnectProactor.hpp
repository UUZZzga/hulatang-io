#ifndef HULATANG_IO_PROACTOR_WSCONNECTPROACTOR_HPP
#define HULATANG_IO_PROACTOR_WSCONNECTPROACTOR_HPP

#include "hulatang/io/Proactor.hpp"

struct sockaddr;

namespace hulatang::io {

// Windows Socket IOCP
class WSConnectProactor : public Proactor
{
public:
    explicit WSConnectProactor(Channel *channel, sockaddr *addr, size_t addrLen);

    void handleRead(Channel *channel, Over *over) override;
    void handleWrite(Channel *channel, Over *over) override;

private:
    void postConnect(Channel *channel, sockaddr *addr, size_t addrLen, Over *over);
};
} // namespace hulatang::io

#endif // HULATANG_IO_PROACTOR_WSCONNECTPROACTOR_HPP
