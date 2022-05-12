#include "hulatang/io/proactor/WSAcceptProactor.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/platform/win32/Type.hpp"
#include "hulatang/io/Channel.hpp"
#include <cstdlib>

#include "hulatang/io/proactor/WindowsOver.hpp"
#include "hulatang/base/error/ErrorCode.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <ws2spi.h>

#include <Mswsock.h>

namespace hulatang::io {
constexpr size_t simultaneousAccepts = 32;
constexpr int addrLen = sizeof(sockaddr_in) + 16; // TODO 没有支持ipv6

struct AcceptOverlapped : public WindowsOver
{
    base::fd_t sockfd;
    int af; // TODO af没设置
    DWORD dwBytes;
    char lpOutputBuf[128];
};

WSAcceptProactor::WSAcceptProactor(Channel *channel, NewConnectionCallback &&newConnectionCallback)
    : newConnectionCallback(std::move(newConnectionCallback))
{
    channel->enableOver();
}

void WSAcceptProactor::handleRead(Channel *channel, Over *over)
{
    // TODO 定位到哪个Over
    // channel 存 Over指针 队列 / 触发的事件
    auto *wsOver = static_cast<AcceptOverlapped *>(over);

    newConnectionCallback(
        wsOver->sockfd, InetAddress::copyFromNative(reinterpret_cast<const sockaddr *>(wsOver->lpOutputBuf + addrLen), addrLen));

    postAccept(channel, wsOver);
}

void WSAcceptProactor::handleWrite(Channel *channel, Over *over)
{
    abort();
}

void WSAcceptProactor::postAccept(Channel *channel)
{
    for (size_t i = 0; i < simultaneousAccepts; ++i)
    {
        auto *wsOver = new AcceptOverlapped{.sockfd = INVALID_SOCKET, .af = AF_INET, .dwBytes = 0, .lpOutputBuf{}};
        postAccept(channel, wsOver);
    }
}

void WSAcceptProactor::postAccept(Channel *channel, Over *over)
{
    auto *wsOver = static_cast<AcceptOverlapped *>(over);
    wsOver->sockfd = WSASocketW(wsOver->af, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == wsOver->sockfd)
    {
        // TODO error_condition 暂时先这样处理
        std::error_condition condition{WSAGetLastError(), win32_category()};
        channel->handleError(condition);

        abort();
    }

    u_long ul = 1;
    if (SOCKET_ERROR == ::ioctlsocket(wsOver->sockfd, FIONBIO, &ul))
    {
        auto ec = make_win32_error_code(WSAGetLastError());
        HLT_CORE_ERROR("ioctlsocket failed, ec = {}, message: {}", ec.value(), ec.message());
        return;
    }

    if (::AcceptEx(channel->getFd().getFd(), wsOver->sockfd, wsOver->lpOutputBuf, 0, addrLen, addrLen, &wsOver->dwBytes,
            &wsOver->overlapped) == FALSE)
    {
        // TODO error_condition 暂时先这样处理
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            std::error_condition condition{err, win32_category()};
            channel->handleError(condition);
            return;
        }
    }
    wsOver->type = Type::READ;
    addOver(channel, wsOver);
}

} // namespace hulatang::io
