#include "hulatang/base/File.hpp"

#include "hulatang/base/Socket.hpp"
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

FileDescriptor::FileDescriptor(socket::fd_t fd)
    : impl(std::make_unique<Impl>(Impl{.fd = fd}))
{}

FileDescriptor::~FileDescriptor() noexcept
{
    if (impl)
    {
        close();
    }
}

FileDescriptor::FileDescriptor(FileDescriptor &&other) noexcept
    : impl(move(other.impl))
{}

socket::fd_t FileDescriptor::getFd() const noexcept
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
    if ((oflag & READ) != 0)
    {
        impl->read = true;
    }
    if ((oflag & WRITE) != 0)
    {
        impl->write = true;
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
    impl = std::make_unique<Impl>(Impl{nullptr});
    impl->fd = fd;
    if ((oflag & READ) != 0)
    {
        impl->read = true;
    }
    if ((oflag & WRITE) != 0)
    {
        impl->write = true;
    }
}

int64_t FileDescriptor::lseek(int64_t offset, int whence, std::error_condition &condition)
{
    assert(impl);
    // TODO ::lseek(int __fd, __off_t __offset, int __whence)
}

void FileDescriptor::socket(sockaddr *addr, size_t len)
{
    assert(!impl);
    int sockfd = ::socket(addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        abort();
    }
#if defined(SO_NOSIGPIPE)
    {
        int on = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
    }
#endif
    {
        int val = fcntl(sockfd, F_GETFL);
        val |= O_NONBLOCK | O_CLOEXEC;
        fcntl(sockfd, F_SETFD, val);
    }
    impl = std::make_unique<Impl>(Impl{nullptr});
    impl->fd = sockfd;
}

void FileDescriptor::bind(sockaddr *addr, size_t len)
{
    assert(impl);

    {
        int on = 1;
        setsockopt(impl->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    }
    if (addr != nullptr)
    {
        if (::bind(impl->fd, addr, len) < 0)
        {
            int err = errno;
            HLT_ERROR("bind err:{}", err);
        }
    }
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
    impl->accept = true;
}

void FileDescriptor::accept(FileDescriptor &fd, std::error_condition &condition)
{
    assert(impl);
    assert(!fd.impl);
    condition = make_file_error_condition(FileErrorCode::CONNECTING);
    // fd.impl = std::make_unique<Impl>(Impl{nullptr});
}

void FileDescriptor::connect(sockaddr *addr, size_t len, std::error_condition &condition) noexcept
{
    assert(impl);
    if (::connect(impl->fd, addr, len) < 0)
    {
        int err = errno;
        if (err == EINPROGRESS)
        {
            condition = make_file_error_condition(FileErrorCode::CONNECTING);
        }
        else
        {
            // TODO 修改为 FileErrorCode
            // condition.assign(err, std::system_category());
            condition = make_file_error_condition(FileErrorCode::CONNECTION_REFUSED);
        }
    }
}

void FileDescriptor::read(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);
    assert((impl->event & READ) == 0);
    impl->read = true;
    impl->readBuf = buf;
}

void FileDescriptor::write(const Buf &buf, std::error_condition &condition) noexcept
{
    assert(impl);
    assert((impl->event & FileDescriptor::Impl::WRITE) == 0);
    auto *implPtr = impl.get();
    implPtr->write = true;
    implPtr->writeBuf = buf;
}

void FileDescriptor::close() noexcept
{
    assert(impl);
    HLT_TRACE("FileDescriptor::close fd: {}", impl->fd);
    if (::close(impl->fd) < 0)
    {
        int err = errno;
        HLT_ERROR("FileDescriptor::close error: {}", err);
    }
    impl.reset();
}

} // namespace hulatang::base