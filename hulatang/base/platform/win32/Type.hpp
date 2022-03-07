#ifndef HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
#define HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP

#include "hulatang/base/File.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <Mswsock.h>

namespace hulatang::base {
using fd_t = uintptr_t;
const fd_t FdNull = static_cast<fd_t>(-1);

union sockaddr_u
{
    sockaddr sa;
    sockaddr_in sin;
    sockaddr_in6 sin6;
};

enum class Type
{
    NONE,
    OPEN,
    CLOSE,
    READ,
    WRITE,
};

struct IO_DATA
{
    OVERLAPPED overlapped;
    Type operationType;
};

struct FileDescriptor::Impl
{
    IO_DATA recvData;
    IO_DATA sendData;
    fd_t fd;
};
} // namespace hulatang::base

#endif // HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
