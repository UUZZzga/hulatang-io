#include "hulatang/base/Socket.hpp"

#include "hulatang/base/Log.hpp"

#include <cerrno>

namespace hulatang::base::socket {
fd_t accept(fd_t listenFd, sockaddr_u *addr, size_t *len)
{
    socklen_t addrLength = 0;
#if HAVE_ACCEPT4
    int connfd = ::accept4(listenFd, &addr->sa, &addrLength, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    int connfd = ::accept(listenFd, reinterpret_cast<sockaddr *>(&addr), &addrLength);
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
        case ECONNABORTED: // 连接已中止
        {
        }
        case EINTR: // 系统调用被在有效连接到达之前捕获的信号中断
        {
        }
        case EMFILE: // 已达到每个进程对打开的文件描述符数量的限制
        case ENFILE: // 已达到系统范围内打开文件总数的限制
        {
        }
        return -err;
        case ENOBUFS:
        case ENOMEM: {
            // 没有足够的可用内存
        }
        case EPROTO:     // 协议错误
        case ENOTSOCK:   // 不是套接字
        case EOPNOTSUPP: // 引用的套接字不是SOCK_STREAM类型
        case EINVAL:     // 套接字未侦听连接，或addrlen无效
        case EFAULT:     // addr参数不在用户地址空间的可写部分
        case EBADF:      // 参数sockfd不是有效的文件描述符
        {
            HLT_FATAL("unknown error of ::accept {}", err);
            abort();
        }
        break;
        default:
            HLT_FATAL("unknown error of ::accept {}", err);
            break;
        }
    }
    if (len != nullptr)
    {
        *len = addrLength;
    }
#ifndef HAVE_ACCEPT4
    int val = fcntl(connfd, F_GETFL);
    val |= O_NONBLOCK | O_CLOEXEC;
    fcntl(connfd, F_SETFD, val);
#endif // !HAVE_ACCEPT4

    return connfd;
}
} // namespace hulatang::base::socket