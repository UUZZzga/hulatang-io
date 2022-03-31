#include "hulatang/base/File.hpp"

#include "hulatang/base/platform/posix/Type.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/extend/Defer.hpp"
#include <asm-generic/errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <netdb.h>

using hulatang::base::EXEC;
using hulatang::base::READ;
using hulatang::base::WRITE;

using hulatang::base::OFlag;

namespace {
inline int buildAccess(OFlag oflag)
{
    int access = 0;
    if ((oflag & READ) != 0 && (oflag & WRITE) != 0)
    {
        access |= O_RDWR;
    }
    if ((oflag & READ) != 0)
    {
        access |= O_RDONLY;
    }
    if ((oflag & WRITE) != 0)
    {
        access |= O_WRONLY;
    }
    access |= O_NONBLOCK | O_CLOEXEC;
    return access;
}
} // namespace

namespace hulatang::base {
FileDescriptor::FileDescriptor() = default;

FileDescriptor::~FileDescriptor() noexcept = default;

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : impl(move(other.impl))
{}

uintptr_t FileDescriptor::getFd() const noexcept
{
    assert(impl);
    return impl->fd;
}

void FileDescriptor::open(std::string_view path, OFlag oflag, std::error_condition &condition)
{
    assert(!impl);
    int fd = ::open(path.data(), buildAccess(oflag));
    if (fd < 0)
    {
        int err = errno;
    }
}

void FileDescriptor::create(std::string_view path, OFlag oflag, std::error_condition &condition)
{
    assert(!impl);
    int fd = ::open(path.data(), buildAccess(oflag) | O_CREAT);
    if (fd < 0)
    {
        int err = errno;
        switch (err)
        {
        case EDQUOT: {
            // 储存空间不够创建文件
        }
        break;
        case EFAULT: {
            // path 指向可访问的地址空间之外
        }
        break;
        }
    }
}

int64_t FileDescriptor::lseek(int64_t offset, int whence, std::error_condition &condition)
{
    assert(impl);
    // TODO ::lseek(int __fd, __off_t __offset, int __whence)
}

void FileDescriptor::bind(sockaddr *addr, size_t len)
{
    assert(!impl);

    int fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (addr != nullptr)
    {
        ::bind(fd, addr, len);
    }

    impl.reset(new Impl{nullptr});
    impl->fd = fd;
}

void FileDescriptor::listen(std::error_condition &condition)
{
    assert(impl);
    if (::listen(impl->fd, SOMAXCONN) < 0)
    {
        int err = errno;
        switch (err)
        {
        case EADDRINUSE: {
            // 端口占用
        }
        break;
        case EBADF:
        // 参数sockfd不是有效的文件描述符
        case ENOTSOCK:
        // 文件描述符sockfd不引用套接字
        case EOPNOTSUPP:
            // 套接字的类型不支持listen 操作
            {
                throw std::runtime_error("Invalid");
            }
            break;
        }
    }
}

void FileDescriptor::accept(FileDescriptor &fd, std::error_condition &condition)
{
    assert(impl);
    assert(!fd.impl);

    sockaddr_in6 addr{};
    socklen_t addrLength = 0;
#if HAVE_ACCEPT4
    int connfd = ::accept4(impl->fd, reinterpret_cast<sockaddr *>(&addr), &addrLength, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    int connfd = ::accept(impl->fd, reinterpret_cast<sockaddr *>(&addr), &addrLength);
#endif
    if (connfd < 0)
    {
        int err = errno;
        switch (err)
        {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        {
            // 没有请求
        }
        break;
        case ECONNABORTED: // 连接已中止
        {
        }
        break;
        case EINTR: // 系统调用被在有效连接到达之前捕获的信号中断
        {
        }
        break;
        case EMFILE: // 已达到每个进程对打开的文件描述符数量的限制
        case ENFILE: // 已达到系统范围内打开文件总数的限制
        {
        }
        break;
        case ENOBUFS:
        case ENOMEM: {
            // 没有足够的可用内存
        }
        break;
        case EPROTO:     // 协议错误
        case ENOTSOCK:   // 不是套接字
        case EOPNOTSUPP: // 引用的套接字不是SOCK_STREAM类型
        case EINVAL:     // 套接字未侦听连接，或addrlen无效
        case EFAULT:     // addr参数不在用户地址空间的可写部分
        case EBADF:      // 参数sockfd不是有效的文件描述符
        {
            throw std::runtime_error("Invalid");
        }
        break;
        default:
            HLT_FATAL("unknown error of ::accept {}", err);
            break;
        }
    }
}

void FileDescriptor::connect(sockaddr *addr, size_t len, std::error_condition &condition) noexcept
{
    assert(impl);
    ::connect(0, addr, len);
}

void FileDescriptor::read(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);
    assert((impl->event & READ) == 0);
    impl->event |= READ;

    ssize_t size = ::read(impl->fd, buf.buf, buf.len);
    if (size < 0)
    {
        int err = errno;
        switch (err)
        {
        case EAGAIN:
#if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
        {
            // 没有请求
        }
        break;

        case EINVAL:
        case EBADF:  // fd不是有效的文件描述符或未打开读取
        case EFAULT: // buf在您可访问的地址空间之外
        case EINTR:  // 在读取任何数据之前，被信号中断
        {
            // TODO
            abort();
        }
        break;
        }
    }
}

void FileDescriptor::write(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);
    assert((impl->event & FileDescriptor::Impl::WRITE) == 0);
    impl->event |= WRITE;
}

void FileDescriptor::close() noexcept
{
    assert(impl);

    if (::close(impl->fd) < 0)
    {
        int err = errno;
        HLT_ERROR("FileDescriptor::close error: {}", err);
    }
    impl.reset();
}

const sockaddr *FileDescriptor::peeraddr()
{
    assert(impl);
}

size_t FileDescriptor::peeraddrLength() {}

} // namespace hulatang::base