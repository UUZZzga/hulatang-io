#ifndef HULATANG_BASE_SOCKET_HPP
#define HULATANG_BASE_SOCKET_HPP

#include "hulatang/base/def.h"

#include <cstdint>

#if HLT_PLATFORM_WINDOWS
#    include <ws2tcpip.h>
#else
#    include <netinet/in.h>
#endif

namespace hulatang::base::socket {
#if HLT_PLATFORM_WINDOWS
using fd_t = uintptr_t;
#else
using fd_t = int;
#endif
const fd_t FdNull = static_cast<fd_t>(-1);

union sockaddr_u
{
    sockaddr sa;
    sockaddr_in sin;
    sockaddr_in6 sin6;
};

fd_t accept(fd_t listenFd, sockaddr_u *addr, size_t* len);
} // namespace hulatang::base::socket

#endif // HULATANG_BASE_SOCKET_HPP
