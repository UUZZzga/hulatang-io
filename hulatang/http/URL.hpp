#ifndef HULATANG_HTTP_URL_HPP
#define HULATANG_HTTP_URL_HPP

#include <string_view>

namespace hulatang::http {
class URL
{
public:
    explicit URL(std::string url);

    explicit URL(std::string_view url)
        : URL(std::string{url})
    {}

    [[nodiscard]] const std::string_view &path() const
    {
        return path_;
    }
    [[nodiscard]] const std::string_view &query() const
    {
        return query_;
    }

    [[nodiscard]] const std::string_view &host() const
    {
        return host_;
    }

    [[nodiscard]] const std::string_view &port() const
    {
        return port_;
    }

    [[nodiscard]] const std::string_view &protocol() const
    {
        return protocol_;
    }

    [[nodiscard]] const std::string_view &ref() const
    {
        return ref_;
    }

private:
    void split();
    std::string url_;
    std::string_view host_;
    std::string_view port_;
    std::string_view protocol_;
    std::string_view path_;
    std::string_view query_;
    std::string_view ref_;
};
} // namespace hulatang::http

#endif // HULATANG_HTTP_URL_HPP
