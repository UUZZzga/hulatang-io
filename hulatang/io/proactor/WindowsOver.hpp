#ifndef HULATANG_IO_PROACTOR_WINDOWSOVER_HPP
#define HULATANG_IO_PROACTOR_WINDOWSOVER_HPP

#include "hulatang/io/Proactor.hpp"

#include <winsock2.h>
#include <iptypes.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>

namespace hulatang::io {
struct WindowsOver : Proactor::Over
{
    OVERLAPPED overlapped{};
    DWORD numberOfBytesTransferred{};
    static const size_t offset;
};
} // namespace hulatang::io

#endif // HULATANG_IO_PROACTOR_WINDOWSOVER_HPP
