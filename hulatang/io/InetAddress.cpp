#include "hulatang/io/InetAddress.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/def.h"

#include "hulatang/base/extend/Defer.hpp"
#include <corecrt_malloc.h>
#include <vcruntime_string.h>

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
} // namespace

namespace hulatang::io {
InetAddress::InetAddress(sockaddr *addr, size_t addrLen)
    : addr_(addr)
    , addrLen_(addrLen)
{}

InetAddress::~InetAddress() noexcept
{
    freeSocketAddress(addr_, addrLen_);
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
    sockaddr *sa = allocSocketAddress(answer->ai_addrlen);
    copySocketAddress(answer->ai_addr, sa, answer->ai_addrlen);
    return {sa, answer->ai_addrlen};
}

InetAddress InetAddress::fromHostnameAndPort(const std::string &host, uint16_t port, bool mandatoryIPv4, bool tcp)
{
    // TODO 没有实现
    return {};
}
} // namespace hulatang::io