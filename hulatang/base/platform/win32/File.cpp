#include "hulatang/base/File.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/base/extend/Defer.hpp"

#include <system_error>
#include <variant>
#include <cassert>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <Mswsock.h>

using hulatang::make_win32_error_code;

using hulatang::base::Buf;
using hulatang::base::O_EXEC;
using hulatang::base::O_READ;
using hulatang::base::O_WRITE;
using hulatang::base::OFlag;

namespace impl {
using fd_t = uintptr_t;
const fd_t FdNull = static_cast<fd_t>(-1);

constexpr uintptr_t socketFlag = 1ULL << 63;

union sockaddr_u
{
    sockaddr sa;
    sockaddr_in sin;
    sockaddr_in6 sin6;
};

sockaddr_u getSockaddr(std::string_view host, int port, std::error_code &ec)
{
    sockaddr_u sa{};

    auto &sin = sa.sin;
    if (host.empty())
    {
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
#ifdef ENABLE_IPV6
        auto &sin6 = sa.sin6;
#endif

        if (inet_pton(AF_INET, host.data(), &sin.sin_addr) == 1)
        {
            sin.sin_family = AF_INET;
        }
#ifdef ENABLE_IPV6
        else if (inet_pton(AF_INET6, host, &sin6.sin6_addr) == 1)
        {
            sin6.sin6_family = AF_INET6;
        }
#endif
        else
        {
            struct addrinfo *answer = nullptr;
            struct addrinfo hint
            {
                .ai_flags = AI_ALL, .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP
            };

            if (getaddrinfo(host.data(), nullptr, &hint, &answer) != 0)
            {
                ec = make_win32_error_code(WSAGetLastError());
                return {};
            }

            for (auto *it = answer; it != nullptr; it = it->ai_next)
            {
                switch (it->ai_family)
                {
                case AF_INET: {
                    auto *ipv4 = reinterpret_cast<struct sockaddr_in *>(it->ai_addr);
                    sin = *ipv4;
                    break;
                }
#ifdef ENABLE_IPV6
                case AF_INET6: {
                    auto *ipv6 = reinterpret_cast<struct sockaddr_in6 *>(it->ai_addr);
                    sin6 = *ipv6;
                    break;
                }
#endif
                }
            }
        }
    }

    if (sa.sa.sa_family == AF_INET)
    {
        sa.sin.sin_port = htons(port);
    }
    else if (sa.sa.sa_family == AF_INET6)
    {
        sa.sin6.sin6_port = htons(port);
    }
    else
    {
        return {};
    }

    return sa;
}

inline DWORD buildAccess(OFlag oflag)
{
    DWORD access = 0;
    if ((oflag & O_READ) != 0)
    {
        access |= GENERIC_READ;
    }
    if ((oflag & O_WRITE) != 0)
    {
        access |= GENERIC_WRITE;
    }
    if ((oflag & O_EXEC) != 0)
    {
        access |= GENERIC_EXECUTE;
    }
    return access;
}

fd_t open(std::string_view path, OFlag oflag, std::error_code &ec) noexcept
{
    std::variant<fd_t, HANDLE> var;
    DWORD access = buildAccess(oflag);
    std::get<HANDLE>(var) = CreateFile(path.data(), access, 0, nullptr, OPEN_EXISTING,
        //   FILE_FLAG_NO_BUFFERING |
        FILE_FLAG_OVERLAPPED, nullptr);
    if (std::get<fd_t>(var) == FdNull)
    {
        ec = make_win32_error_code(static_cast<int>(GetLastError()));
        return {};
    }
    return std::get<fd_t>(var);
}

fd_t bind(std::string_view localHost, int localPort, std::error_code &ec) noexcept
{
    fd_t fd = FdNull;
    sockaddr_u localAddr = getSockaddr(localHost, localPort, ec);
    if (ec)
    {
        return FdNull;
    }
    int addrSize = localAddr.sa.sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);

    fd = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == fd)
    {
        return FdNull;
    }

    auto error = [&](int err = -1) {
        closesocket(fd);
        if (err == -1)
        {
            err = WSAGetLastError();
        }
        ec = make_win32_error_code(err);
        return FdNull;
    };

    LPFN_CONNECTEX ConnectEx = nullptr;
    DWORD dwBytes = 0;
    GUID guidConnectEx = WSAID_CONNECTEX;
    if (SOCKET_ERROR == WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), &ConnectEx,
                            sizeof(ConnectEx), &dwBytes, nullptr, nullptr))
    {
        return error();
    }

    if (SOCKET_ERROR == bind(fd, &localAddr.sa, addrSize))
    {
        int err = WSAGetLastError();
        if (ERROR_IO_PENDING != err)
        {
            return error(err);
        }
    }
    assert((fd & socketFlag) == 0);
    return fd | socketFlag;
}

void connect(fd_t fd, std::string_view peerHost, int peerPort, OVERLAPPED *overlapped, std::error_code &ec) noexcept
{
    assert(INVALID_SOCKET != fd);

    int addrSize = 0;

    sockaddr_u peerAddr = getSockaddr(peerHost, peerPort, ec);

    LPFN_CONNECTEX ConnectEx = nullptr;

    if (ConnectEx(fd, &peerAddr.sa, addrSize, nullptr, 0, nullptr, overlapped) == FALSE)
    {
        ec = make_win32_error_code(WSAGetLastError());
    }
}

void read(fd_t fd, const Buf &buf, OVERLAPPED *overlapped, std::error_code &ec) noexcept
{
    WSABUF wsabuf{.len = static_cast<ULONG>(buf.len), .buf = buf.buf};
    DWORD flags = 0;
    if (SOCKET_ERROR == WSARecv(fd, &wsabuf, 1, nullptr, &flags, overlapped, nullptr))
    {
        ec = make_win32_error_code(WSAGetLastError());
    }
}

void write(fd_t fd, const Buf &buf, OVERLAPPED *overlapped, std::error_code &ec) noexcept
{
    WSABUF wsabuf{.len = static_cast<ULONG>(buf.len), .buf = buf.buf};
    DWORD flags = 0;

    if (SOCKET_ERROR == WSASend(fd, &wsabuf, 1, nullptr, flags, overlapped, nullptr))
    {
        ec = make_win32_error_code(WSAGetLastError());
    }
}

void close(fd_t fd) noexcept
{
    if ((fd & socketFlag) == 0)
    {
        close(fd & (~socketFlag));
    }
    else
    {
        closesocket(fd & (~socketFlag));
    }
}
} // namespace impl

namespace hulatang::base {
struct FileDescriptor::Impl
{
    OVERLAPPED recvOverlapped;
    OVERLAPPED sendOverlapped;
    impl::fd_t fd;
};

FileDescriptor::FileDescriptor() = default;

FileDescriptor::~FileDescriptor() noexcept = default;

void FileDescriptor::open(std::string_view path, OFlag oflag)
{
    assert(!impl);
    std::error_code ec;
    impl::fd_t fd = impl::open(path, oflag, ec);
    if (ec)
    {
        return;
    }
    impl = std::make_unique<Impl>();
    impl->fd = fd;
}

void FileDescriptor::bind(std::string_view localHost, int localPort)
{
    assert(!impl);
    std::error_code ec;
    impl::fd_t fd = impl::bind(localHost, localPort, ec);
    if (ec)
    {
        return;
    }
    impl = std::make_unique<Impl>();
    impl->fd = fd;
}

void FileDescriptor::connect(std::string_view peerHost, int peerPort) noexcept
{
    assert(impl);

    std::error_code ec;
    impl::connect(impl->fd, peerHost, peerPort, &impl->recvOverlapped, ec);
    if (!ec)
    {
        return;
    }

    // 处理错误
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::read(const Buf &buf, std::error_condition &condition) noexcept
{
    std::error_code ec;
    impl::read(impl->fd, buf, &impl->recvOverlapped, ec);
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::write(const Buf &buf, std::error_condition &condition) noexcept
{
    std::error_code ec;
    impl::write(impl->fd, buf, &impl->sendOverlapped, ec);
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::close() noexcept
{
    impl::close(impl->fd);
}

} // namespace hulatang::base