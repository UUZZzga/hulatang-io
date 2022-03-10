#ifndef HULATANG_IO_INETADDRESS_HPP
#define HULATANG_IO_INETADDRESS_HPP

#include <memory>
#include <string_view>

struct sockaddr;

namespace hulatang::io {

class InetAddress
{
public:
    InetAddress() noexcept = default;
    ~InetAddress() noexcept;

    InetAddress(InetAddress &&other) noexcept
    {
        swap(other);
    }

    InetAddress &operator=(InetAddress &&other) noexcept
    {
        InetAddress temp(std::move(other));
        swap(temp);
        return *this;
    }

    InetAddress(const InetAddress &other) = delete;
    InetAddress &operator=(const InetAddress &other) = delete;

    static InetAddress fromHostnameAndService(const std::string &host, const std::string &service, bool mandatoryIPv4 = true);
    static InetAddress fromHostnameAndPort(const std::string &host, uint16_t port, bool mandatoryIPv4 = true, bool tcp = true);

    void swap(InetAddress &other) noexcept
    {
        std::swap(addr_, other.addr_);
        std::swap(addrLen_, other.addrLen_);
    }

    sockaddr *addr() const { return addr_; }

    size_t addrLen() const { return addrLen_; }

private:
    InetAddress(sockaddr *addr, size_t addrLen);

    sockaddr *addr_{};
    size_t addrLen_{};
};
} // namespace hulatang::io

#endif // HULATANG_IO_INETADDRESS_HPP
