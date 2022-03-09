#include "hulatang/base/File.hpp"

#include "hulatang/base/platform/win32/Type.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/base/extend/Defer.hpp"

#include <system_error>
#include <variant>
#include <cassert>

using hulatang::make_win32_error_code;

using hulatang::base::Buf;
using hulatang::base::O_EXEC;
using hulatang::base::O_READ;
using hulatang::base::O_WRITE;
using hulatang::base::OFlag;

constexpr uintptr_t socketFlag = 1ULL << 63;

namespace impl {
using hulatang::base::fd_t;
using hulatang::base::FdNull;
using hulatang::base::IO_DATA;
using hulatang::base::sockaddr_u;

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
    union
    {
        fd_t fd;
        HANDLE handle;
    };
    DWORD access = buildAccess(oflag);
    handle = CreateFile(path.data(), access, 0, nullptr, OPEN_EXISTING,
        //   FILE_FLAG_NO_BUFFERING |
        FILE_FLAG_OVERLAPPED, nullptr);
    if (fd == FdNull)
    {
        ec = make_win32_error_code(static_cast<int>(GetLastError()));
        return {};
    }
    return fd;
}

fd_t create(std::string_view path, OFlag oflag, std::error_code &ec) noexcept
{
    union
    {
        fd_t fd;
        HANDLE handle;
    };
    DWORD access = buildAccess(oflag);
    handle = CreateFile(path.data(), access, 0, nullptr, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr);
    if (fd == FdNull)
    {
        ec = make_win32_error_code(static_cast<int>(GetLastError()));
        return {};
    }
    return fd;
}

int64_t lseek(fd_t fd, int64_t offset, int whence, std::error_code &ec) noexcept
{
    if ((fd & socketFlag) != 0U)
    {
        abort();
    }
    LARGE_INTEGER move = {.QuadPart = offset};
    LARGE_INTEGER li = {.QuadPart = 0};

    BOOL bRet = SetFilePointerEx(reinterpret_cast<HANDLE>(fd), move, &li, whence);
    if (bRet == FALSE)
    {
        ec = make_win32_error_code(GetLastError());
        return -1;
    }
    return li.QuadPart;
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

void listen(fd_t fd, std::error_code &ec)
{
    if ((fd & socketFlag) != 0U)
    {
        fd = fd & (~socketFlag);
        if (0 != ::listen(fd, SOMAXCONN))
        {
            ec = make_win32_error_code(WSAGetLastError());
        }
    }
    else
    {
        abort();
    }
}

void accept(fd_t listen, fd_t &accept, char *buf, IO_DATA &data, std::error_code &ec)
{
    if ((listen & socketFlag) != 0U)
    {
        DWORD dwBytes = 0;
        listen = listen & (~socketFlag);
        accept = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (INVALID_SOCKET == accept)
        {
            ec = make_win32_error_code(WSAGetLastError());
            return;
        }
        int addrLen = sizeof(sockaddr_in) + 16; // TODO 没有支持ipv6
        if (AcceptEx(listen, accept, buf, 0, addrLen, addrLen, &dwBytes, &data.overlapped) == FALSE)
        {
            ec = make_win32_error_code(WSAGetLastError());
        }
        data.operationType = hulatang::base::Type::READ;
        accept |= socketFlag;
    }
    else
    {
        abort();
    }
}

void connect(fd_t fd, std::string_view peerHost, int peerPort, IO_DATA &data, std::error_code &ec) noexcept
{
    assert(INVALID_SOCKET != fd);
    assert(fd & socketFlag);
    fd = fd & (~socketFlag);
    int addrSize = 0;

    sockaddr_u peerAddr = getSockaddr(peerHost, peerPort, ec);
    if (peerAddr.sa.sa_family == AF_INET)
    {
        addrSize = sizeof(sockaddr_in);
    }
    else if (peerAddr.sa.sa_family == AF_INET6)
    {
        addrSize = sizeof(sockaddr_in6);
    }

    LPFN_CONNECTEX ConnectEx = nullptr;
    DWORD dwBytes = 0;
    GUID guidConnectEx = WSAID_CONNECTEX;
    if (SOCKET_ERROR == WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), &ConnectEx,
                            sizeof(ConnectEx), &dwBytes, nullptr, nullptr))
    {
        ec = make_win32_error_code(WSAGetLastError());
        return;
    }

    if (ConnectEx(fd, &peerAddr.sa, addrSize, nullptr, 0, nullptr, &data.overlapped) == FALSE)
    {
        ec = make_win32_error_code(WSAGetLastError());
        return;
    }
    data.operationType = hulatang::base::Type::OPEN;
}

void read(fd_t fd, const Buf &buf, IO_DATA &data, std::error_code &ec) noexcept
{
    if ((fd & socketFlag) != 0U)
    {
        fd = fd & (~socketFlag);

        WSABUF wsabuf{.len = static_cast<ULONG>(buf.len), .buf = buf.buf};
        data.buf = buf.buf;
        DWORD flags = 0;
        if (SOCKET_ERROR == WSARecv(fd, &wsabuf, 1, nullptr, &flags, &data.overlapped, nullptr))
        {
            ec = make_win32_error_code(WSAGetLastError());
        }
    }
    else
    {
        LARGE_INTEGER fileSize;
        LARGE_INTEGER filePointer;
        data.buf = buf.buf;
        size_t bufLen = buf.len;
        BOOL bRet = ::GetFileSizeEx(reinterpret_cast<HANDLE>(fd), &fileSize);
        if (bRet == 0)
        {
            ec = make_win32_error_code(GetLastError());
            return;
        }
        filePointer.QuadPart = reinterpret_cast<uintptr_t>(data.overlapped.Pointer);

        if (filePointer.QuadPart > fileSize.QuadPart)
        {
            bufLen = 0;
        }
        else if (filePointer.QuadPart + static_cast<LONGLONG>(buf.len) > fileSize.QuadPart)
        {
            bufLen = fileSize.QuadPart - filePointer.QuadPart;
        }

        bRet = ::ReadFile(reinterpret_cast<HANDLE>(fd), buf.buf, bufLen, nullptr, &data.overlapped);
        if (bRet == 0)
        {
            ec = make_win32_error_code(GetLastError());
        }
    }
    data.operationType = hulatang::base::Type::READ;
}

void write(fd_t fd, const Buf &buf, IO_DATA &data, std::error_code &ec) noexcept
{
    if ((fd & socketFlag) != 0U)
    {
        fd = fd & (~socketFlag);

        WSABUF wsabuf{.len = static_cast<ULONG>(buf.len), .buf = buf.buf};
        DWORD flags = 0;
        data.buf = buf.buf;

        if (SOCKET_ERROR == WSASend(fd, &wsabuf, 1, nullptr, flags, &data.overlapped, nullptr))
        {
            ec = make_win32_error_code(WSAGetLastError());
        }
    }
    else
    {
        data.buf = buf.buf;
        BOOL bRet = ::WriteFile(reinterpret_cast<HANDLE>(fd), buf.buf, static_cast<DWORD>(buf.len), nullptr, &data.overlapped);
        if (bRet == 0)
        {
            ec = make_win32_error_code(GetLastError());
        }
    }
    data.operationType = hulatang::base::Type::WRITE;
}

void close(fd_t fd) noexcept
{
    if ((fd & socketFlag) == 0)
    {
        ::CloseHandle(reinterpret_cast<HANDLE>(fd));
    }
    else
    {
        closesocket(fd & (~socketFlag));
    }
}
} // namespace impl

namespace hulatang::base {
FileDescriptor::FileDescriptor() = default;

FileDescriptor::~FileDescriptor() noexcept = default;

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : impl(move(other.impl))
{}

uintptr_t FileDescriptor::getFd() const noexcept
{
    assert(impl);
    return impl->fd & (~socketFlag);
}

void FileDescriptor::open(std::string_view path, OFlag oflag, std::error_condition &condition)
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

void FileDescriptor::create(std::string_view path, OFlag oflag, std::error_condition &condition)
{
    assert(!impl);
    std::error_code ec;
    impl::fd_t fd = impl::create(path, oflag, ec);
    if (ec)
    {
        HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
        return;
    }
    impl = std::make_unique<Impl>();
    impl->fd = fd;
}

int64_t FileDescriptor::lseek(int64_t offset, int whence, std::error_condition &condition)
{
    assert(impl);
    std::error_code ec;
    int64_t o = impl::lseek(impl->fd, offset, whence, ec);
    if (!ec)
    {
        return o;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::updatePosition(uint64_t position)
{
    assert(impl);
#if HLT_PLATFORM == 64
    impl->recvData.overlapped.Pointer = static_cast<LONGLONG>(position);
    impl->sendData.overlapped.Pointer = static_cast<LONGLONG>(position);
#elif HLT_PLATFORM == 32
    LARGE_INTEGER pos = {.QuadPart = static_cast<LONGLONG>(position)};
    impl->recvData.overlapped.Offset = pos.LowPart;
    impl->recvData.overlapped.OffsetHigh = pos.HighPart;
    impl->sendData.overlapped.Offset = pos.LowPart;
    impl->sendData.overlapped.OffsetHigh = pos.HighPart;
#endif
}

void FileDescriptor::bind(std::string_view localHost, int localPort)
{
    assert(!impl);
    std::error_code ec;
    impl::fd_t fd = impl::bind(localHost, localPort, ec);
    if (ec)
    {
        HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
        return;
    }
    impl = std::make_unique<Impl>();
    impl->fd = fd;
}

void FileDescriptor::listen(std::error_condition &condition)
{
    assert(impl);
    std::error_code ec;
    impl::listen(impl->fd, ec);
}

void FileDescriptor::accept(FileDescriptor &fd, std::error_condition &condition)
{
    assert(impl);
    assert(!fd.impl);
    fd.impl = std::make_unique<Impl>();
    std::error_code ec;
    impl::accept(impl->fd, fd.impl->fd, impl->lpOutputBuf, impl->recvData, ec);
    if (!ec)
    {
        fd.impl->recvData.operationType = hulatang::base::Type::OPEN;
        return;
    }
    // 处理错误
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        condition = make_file_error_condition(FileErrorCode::CONNECTING);
        fd.impl->recvData.operationType = hulatang::base::Type::OPEN;
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
    fd.close();
    fd.impl.reset();
}

void FileDescriptor::connect(std::string_view peerHost, int peerPort, std::error_condition &condition) noexcept
{
    assert(impl);

    std::error_code ec;
    impl::connect(impl->fd, peerHost, peerPort, impl->recvData, ec);
    if (!ec)
    {
        return;
    }

    // 处理错误
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        condition = make_file_error_condition(FileErrorCode::CONNECTING);
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::read(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);

    std::error_code ec;
    impl::read(impl->fd, buf, impl->recvData, ec);
    if (!ec)
    {
        return;
    }
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::write(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);
    std::error_code ec;
    impl::write(impl->fd, buf, impl->sendData, ec);
    if (!ec)
    {
        return;
    }
    if (ec.category() == win32_category() && ERROR_IO_PENDING == ec.value())
    {
        return;
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
}

void FileDescriptor::close() noexcept
{
    assert(impl);
    if (impl->recvData.operationType != Type::NONE)
    {
        CancelIoEx(reinterpret_cast<HANDLE>(impl->fd), &(impl->recvData.overlapped));
        impl->recvData.operationType = Type::NONE;
    }
    if (impl->sendData.operationType != Type::NONE)
    {
        CancelIoEx(reinterpret_cast<HANDLE>(impl->fd), &(impl->sendData.overlapped));
        impl->sendData.operationType = Type::NONE;
    }

    impl::close(impl->fd);
    impl.reset();
}

} // namespace hulatang::base