#include "hulatang/io/proactor/WSConnectProactor.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/extend/Cast.hpp"

#include "hulatang/io/Channel.hpp"
#include "hulatang/io/Model.hpp"

#include "hulatang/io/SocketModelFactory.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/io/proactor/WindowsOver.hpp"
#include <minwindef.h>
#include <stdlib.h>
#include <system_error>
#include <winerror.h>

namespace hulatang::io {
struct ConnectOver : public WindowsOver
{};

std::unique_ptr<Model> SocketModelFactory::createConnect(Channel *channel, sockaddr *addr, size_t addrLen)
{
    auto connect = std::make_unique<WSConnectProactor>(channel, addr, addrLen);
    return connect;
}

WSConnectProactor::WSConnectProactor(Channel *channel, sockaddr *addr, size_t addrLen)
{
    channel->enableOver();
    postConnect(channel, addr, addrLen, new WindowsOver());
}

void WSConnectProactor::handleRead(Channel *channel, Over *over)
{
    abort();
}

void WSConnectProactor::handleWrite(Channel *channel, Over *over)
{
    auto *wsOver = static_cast<ConnectOver *>(over);
    DWORD numberOfBytesTransferred = 0;
    if (FALSE == ::GetOverlappedResultEx(
                     reinterpret_cast<HANDLE>(channel->getFd().getFd()), &wsOver->overlapped, &numberOfBytesTransferred, 0, FALSE))
    {
    //     auto ec = make_win32_error_code(WSAGetLastError());
    //     HLT_CORE_ERROR("error code: {}, message: {}", ec.value(), ec.message());
    //     abort();
    // }
    int err = WSAGetLastError();
    if (err != 0)
    {
        std::error_code ec;
        switch (err)
        {
        case WSAEFAULT: {
            ec = std::make_error_code(std::errc::bad_address);
        }
        break;
        case WSAEINVAL: {
            ec = std::make_error_code(std::errc::invalid_argument);
        }
        break;
        case WSAENOTSOCK: {
            ec = std::make_error_code(std::errc::not_a_socket);
        }
        break;
        case WSAEAFNOSUPPORT: {
            ec = std::make_error_code(std::errc::address_family_not_supported);
        }
        break;
        case WSAEADDRINUSE: {
            ec = std::make_error_code(std::errc::address_in_use);
        }
        break;
        case WSAENETDOWN: {
            ec = std::make_error_code(std::errc::network_down);
        }
        break;
        case WSAEADDRNOTAVAIL: {
            ec = std::make_error_code(std::errc::address_not_available);
        }
        break;
        case WSAEALREADY: {
            ec = std::make_error_code(std::errc::connection_already_in_progress);
        }
        break;
        case WSAENETUNREACH: {
            // 可以在同一个fd上重连的
            ec = std::make_error_code(std::errc::network_unreachable);
        }
        break;
        case WSAENOBUFS: {
            ec = std::make_error_code(std::errc::no_buffer_space);
        }
        break;
        case WSAEISCONN: {
            ec = std::make_error_code(std::errc::already_connected);
        }
        break;
        case WSAETIMEDOUT: {
            // 可以在同一个fd上重连的
            ec = std::make_error_code(std::errc::timed_out);
        }
        break;
        case WSAECONNREFUSED: {
            // 可以在同一个fd上重连的
            ec = std::make_error_code(std::errc::connection_refused);
        }
        break;
        case WSAEHOSTUNREACH: {
            ec = std::make_error_code(std::errc::host_unreachable);
        }
        break;
        default: {
            ec = make_win32_error_code(err);
        }
        }
        channel->handleError(ec.default_error_condition());
        return;
    }}

    channel->handleWrite();
}

void WSConnectProactor::postConnect(Channel *channel, sockaddr *addr, size_t addrLen, Over *over)
{
    auto *wsOver = static_cast<ConnectOver *>(over);
    LPFN_CONNECTEX ConnectEx = nullptr;
    DWORD dwBytes = 0;
    GUID guidConnectEx = WSAID_CONNECTEX;
    if (SOCKET_ERROR == WSAIoctl(channel->getFd().getFd(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx),
                            &ConnectEx, sizeof(ConnectEx), &dwBytes, nullptr, nullptr))
    {
        auto ec = make_win32_error_code(WSAGetLastError());
        abort();
    }

    if (ConnectEx(channel->getFd().getFd(), addr, implicit_cast<int>(addrLen), nullptr, 0, nullptr, &wsOver->overlapped) == FALSE)
    {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            auto ec = make_win32_error_code(err);
            channel->handleError(ec.default_error_condition());
            return;
        }
    }

    wsOver->type = Type::WRITE;
    addOver(channel, wsOver);
}

} // namespace hulatang::io
