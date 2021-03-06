#ifndef HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
#define HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP

#include "hulatang/base/Socket.hpp"

#include "hulatang/base/File.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <ws2spi.h>

#include <Mswsock.h>

namespace hulatang::base {
using socket::fd_t;
using socket::FdNull;
using socket::sockaddr_u;

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
    Buf buf;
    Type operationType;
};

struct FileDescriptor::Impl
{
    IO_DATA recvData;
    IO_DATA sendData;
    fd_t fd;
    char lpOutputBuf[128];
    DWORD dwBytes;
    int af;
};
} // namespace hulatang::base

#endif // HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
