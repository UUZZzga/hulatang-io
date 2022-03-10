#include "hulatang/io/InetAddress.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/def.h"

#include "hulatang/base/extend/Defer.hpp"
#include <corecrt_malloc.h>
#include <vcruntime_string.h>
#include <winsock2.h>

#if defined(HLT_PLATFORM_WINDOWS)
#    include <ws2tcpip.h>
#endif

namespace {
sockaddr *allocSocketAddress(size_t len)
{
    return (sockaddr *)malloc(len);
}

void freeSocketAddress(sockaddr *addr, size_t len)
{
    ::free(addr);
}

void copySocketAddress(const sockaddr *src, sockaddr *des, size_t len)
{
    memcpy(des, src, len);
}

std::string sock_ntop(const sockaddr *sa, size_t len)
{
    std::array<char, 128> buf{};
    switch (sa->sa_family)
    {
    case AF_INET: {
        const auto *sin = reinterpret_cast<const sockaddr_in *>(sa);
        if (inet_ntop(AF_INET, &sin->sin_addr, buf.data(), buf.size()) == nullptr)
        {
            return {};
        }
        if (ntohs(sin->sin_port) != 0)
        {
            return fmt::format("{}:{}", buf.data(), ntohs(sin->sin_port));
        }
    }
    break;
    case AF_INET6:
        const auto *sin6 = reinterpret_cast<const sockaddr_in6 *>(sa);
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, buf.data(), buf.size()) == nullptr)
        {
            return {};
        }
        if (ntohs(sin6->sin6_port) != 0)
        {
            return fmt::format("[{}]:{}", buf.data(), ntohs(sin6->sin6_port));
        }
    }
    throw std::invalid_argument("Invalid sa_family");
}
} // namespace

namespace hulatang::io {
InetAddress::InetAddress(struct sockaddr *addr, size_t addrLen)
    : sockaddr_(addr)
    , sockaddrLength_(addrLen)
{}

InetAddress::~InetAddress() noexcept
{
    freeSocketAddress(sockaddr_, sockaddrLength_);
}

// 为什么不适用 string_view: 因为string_view 不保证c风格str，也就是'\0'结尾
InetAddress InetAddress::fromHostnameAndService(const std::string &host, const std::string &service, bool mandatoryIPv4)
{
    addrinfo hint{};
    addrinfo *answer = nullptr;
    hint.ai_family = mandatoryIPv4 ? AF_INET : AF_UNSPEC;
    if (int err = ::getaddrinfo(host.c_str(), service.c_str(), &hint, &answer); err != 0)
    {
        HLT_CORE_ERROR("{}", gai_strerror(err));
        return {};
    }
    defer[&]
    {
        ::freeaddrinfo(answer);
    };
    struct sockaddr *sa = allocSocketAddress(answer->ai_addrlen);
    copySocketAddress(answer->ai_addr, sa, answer->ai_addrlen);
    return {sa, answer->ai_addrlen};
}

InetAddress InetAddress::fromHostnameAndPort(const std::string &host, uint16_t port, bool mandatoryIPv4, bool tcp)
{
    // TODO 没有实现
    return {};
}

std::string InetAddress::toString() const
{
    return sock_ntop(sockaddr_, sockaddrLength_);
}
} // namespace hulatang::io