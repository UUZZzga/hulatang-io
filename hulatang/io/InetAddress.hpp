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

    static InetAddress copyFromNative(const struct sockaddr *addr, size_t addrLen);
    static InetAddress fromHostnameAndService(const std::string &host, const std::string &service, bool mandatoryIPv4 = true);
    static InetAddress fromHostnameAndPort(const std::string &host, uint16_t port, bool mandatoryIPv4 = true, bool tcp = true);

    void swap(InetAddress &other) noexcept
    {
        std::swap(sockaddr_, other.sockaddr_);
        std::swap(sockaddrLength_, other.sockaddrLength_);
    }

    [[nodiscard]] sockaddr *getSockaddr() const { return sockaddr_; }

    [[nodiscard]] size_t sockaddrLength() const { return sockaddrLength_; }

    [[nodiscard]] std::string toString() const;

private:
    InetAddress(struct sockaddr *addr, size_t addrLen);

    struct sockaddr *sockaddr_{};
    size_t sockaddrLength_{};
};
} // namespace hulatang::io

#endif // HULATANG_IO_INETADDRESS_HPP
