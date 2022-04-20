#include "hulatang/base/File.hpp"

#include "hulatang/base/extend/Cast.hpp"
#include "hulatang/base/platform/win32/Type.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/base/extend/Defer.hpp"

#include <system_error>
#include <variant>
#include <cassert>

using hulatang::make_win32_error_code;

using hulatang::base::Buf;
using hulatang::base::EXEC;
using hulatang::base::OFlag;
using hulatang::base::READ;
using hulatang::base::WRITE;

constexpr uintptr_t socketFlag = 1ULL << 63;

namespace impl {
constexpr int addrLen = sizeof(sockaddr_in) + 16; // TODO 没有支持ipv6

using hulatang::implicit_cast;
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
    if ((oflag & READ) != 0)
    {
        access |= GENERIC_READ;
    }
    if ((oflag & WRITE) != 0)
    {
        access |= GENERIC_WRITE;
    }
    if ((oflag & EXEC) != 0)
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

void bind(fd_t fd, sockaddr *addr, size_t len, std::error_code &ec) noexcept
{
    assert(fd & socketFlag);
    fd  = fd & (~socketFlag);

    if (addr != nullptr)
    {
        if (SOCKET_ERROR == bind(fd, addr, implicit_cast<DWORD>(len)))
        {
            int err = WSAGetLastError();
            closesocket(fd);
            ec = make_win32_error_code(err);
            return;
        }
    }
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

void accept(fd_t listen, fd_t &accept, int af, char *buf, IO_DATA &data, std::error_code &ec)
{
    if ((listen & socketFlag) != 0U)
    {
        DWORD dwBytes = 0;
        listen = listen & (~socketFlag);
        accept = WSASocketW(af, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (INVALID_SOCKET == accept)
        {
            ec = make_win32_error_code(WSAGetLastError());
            return;
        }
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

void connect(fd_t fd, sockaddr *addr, size_t len, IO_DATA &data, std::error_code &ec) noexcept
{
    assert(INVALID_SOCKET != fd);
    assert(fd & socketFlag);
    fd = fd & (~socketFlag);

    LPFN_CONNECTEX ConnectEx = nullptr;
    DWORD dwBytes = 0;
    GUID guidConnectEx = WSAID_CONNECTEX;
    if (SOCKET_ERROR == WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx), &ConnectEx,
                            sizeof(ConnectEx), &dwBytes, nullptr, nullptr))
    {
        ec = make_win32_error_code(WSAGetLastError());
        return;
    }

    if (ConnectEx(fd, addr, static_cast<int>(len), nullptr, 0, nullptr, &data.overlapped) == FALSE)
    {
        ec = make_win32_error_code(WSAGetLastError());
    }
    data.operationType = hulatang::base::Type::OPEN;
}

void read(fd_t fd, const Buf &buf, IO_DATA &data, std::error_code &ec) noexcept
{
    if ((fd & socketFlag) != 0U)
    {
        fd = fd & (~socketFlag);

        WSABUF wsabuf{.len = static_cast<ULONG>(buf.len), .buf = buf.buf};
        data.buf = buf;
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
        data.buf = buf;
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

        bRet = ::ReadFile(reinterpret_cast<HANDLE>(fd), buf.buf, implicit_cast<DWORD>(bufLen), nullptr, &data.overlapped);
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
        data.buf = buf;

        if (SOCKET_ERROR == WSASend(fd, &wsabuf, 1, nullptr, flags, &data.overlapped, nullptr))
        {
            ec = make_win32_error_code(WSAGetLastError());
        }
    }
    else
    {
        data.buf = buf;
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

FileDescriptor::~FileDescriptor() noexcept {
    if (impl) {
        close();
    }
}

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
    return -1;
}

void FileDescriptor::updatePosition(uint64_t position)
{
    assert(impl);
#if HLT_ARCH == 64
    impl->recvData.overlapped.Pointer = reinterpret_cast<PVOID>(position);
    impl->sendData.overlapped.Pointer = reinterpret_cast<PVOID>(position);
#elif HLT_ARCH == 32
    LARGE_INTEGER pos = {.QuadPart = static_cast<LONGLONG>(position)};
    impl->recvData.overlapped.Offset = pos.LowPart;
    impl->recvData.overlapped.OffsetHigh = pos.HighPart;
    impl->sendData.overlapped.Offset = pos.LowPart;
    impl->sendData.overlapped.OffsetHigh = pos.HighPart;
#endif
}

void FileDescriptor::socket(sockaddr *addr, size_t len)
{
    assert(!impl);
    fd_t fd = FdNull;
    fd = WSASocketW(addr->sa_family, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == fd)
    {
        return;
    }
    assert((fd & socketFlag) == 0);
    fd = fd | socketFlag;
    impl = std::make_unique<Impl>();
    impl->fd = fd;
}

void FileDescriptor::bind(sockaddr *addr, size_t len)
{
    assert(impl);
    std::error_code ec;
    impl::bind(impl->fd, addr, len, ec);
    if (ec)
    {
        HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
        return;
    }
    impl->af = addr->sa_family;
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
    impl::accept(impl->fd, fd.impl->fd, fd.impl->af, impl->lpOutputBuf, impl->recvData, ec);
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

void FileDescriptor::connect(sockaddr *addr, size_t len, std::error_condition &condition) noexcept
{
    assert(impl);

    std::error_code ec;
    impl::connect(impl->fd, addr, len, impl->recvData, ec);
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
    impl->recvData.operationType = Type::NONE;
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
    if (ec.category() == win32_category())
    {
        switch (ec.value())
        {
        case ERROR_IO_PENDING: {
            assert(impl->recvData.operationType != Type::NONE);
            return;
        }
        case WSAECONNABORTED: {
            // 收到FIN后读取
            // unix下是read返回0
            // 
            // 读取的流程是
            // 先判断 num == 0 关闭连接
            // FileDescriptor::read 也就是本函数
            // 最后执行 messageCallback
            // 不能立刻
            condition = make_file_error_condition(FileErrorCode::EEOF);
            return;
        }
        case WSAECONNRESET: {
            condition = make_file_error_condition(FileErrorCode::RESET);
            return;
        }
        }
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
    impl->recvData.operationType = Type::NONE;
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
    if (ec.category() == win32_category())
    {
        switch (ec.value())
        {
        case ERROR_IO_PENDING: {
            assert(impl->sendData.operationType != Type::NONE);
            return;
        }
        case WSAECONNABORTED:
            // 接收方从不确认（ACK）在数据流套接字上发送的数据
            // 也就是超时
            // 
            // 此时不能写入、不能读取
            // 最好的办法是尽快关闭
        case WSAECONNRESET: {
            condition = make_file_error_condition(FileErrorCode::RESET);
            return;
        }
        }
    }
    HLT_CORE_WARN("error code: {}, message: {}", ec.value(), ec.message());
    impl->sendData.operationType = Type::NONE;
    abort();
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

const sockaddr *FileDescriptor::peeraddr()
{
    assert(impl);
    return reinterpret_cast<const sockaddr *>(impl->lpOutputBuf + impl::addrLen);
}

size_t FileDescriptor::peeraddrLength()
{
    return sizeof(sockaddr_in);
}

} // namespace hulatang::base