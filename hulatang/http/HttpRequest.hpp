#ifndef HULATANG_HTTP_HTTPREQUEST_HPP
#define HULATANG_HTTP_HTTPREQUEST_HPP

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <ostream>

namespace hulatang::http {
class HttpRequest
{
public:
    using HeaderMap = std::unordered_map<std::string, std::string>;

    enum Method
    {
        INVALID,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE
    };
    enum Version
    {
        kUnknown,
        kHttp10,
        kHttp11
    };

    HttpRequest();

    void addHeader(std::string_view name, std::string_view value);

    void addHeader(const char *start, const char *colon, const char *end);

    [[nodiscard]] std::string getHeader(const std::string &field) const;

    [[nodiscard]] const HeaderMap &headers() const
    {
        return *headers_;
    }

    void swap(HttpRequest &that) noexcept;

    [[nodiscard]] Version version() const
    {
        return version_;
    }

    [[nodiscard]] std::string_view versionString() const;

    /**
     * @brief Set the Version From string
     *
     * @param version "HTTP/1.1"
     * @return true
     * @return false version() == kUnknown
     */
    bool setVersionFromString(std::string_view version);

    void setVersion(const Version &version)
    {
        version_ = version;
    }

    [[nodiscard]] Method method() const
    {
        return method_;
    }
    void setMethod(const Method &method) { method_ = method; }

    [[nodiscard]] std::string_view methodString() const;

    /**
     * @brief Set the Method From string
     *
     * @param method Upper Eg. "GET"
     * @return true  Success
     * @return false method() == kInvalid
     */
    bool setMethodFromString(std::string_view method);

    [[nodiscard]] const std::string &path() const
    {
        return path_;
    }
    void setPath(std::string_view path);

    [[nodiscard]] const std::string &query() const
    {
        return query_;
    }

    friend std::ostream &operator<<(std::ostream &out, const HttpRequest &request);

private:
    Method method_;
    Version version_;
    std::chrono::milliseconds receiveTime_;
    std::unique_ptr<HeaderMap> headers_;
    std::string path_;
    std::string query_;
};
} // namespace hulatang::http

#endif // HULATANG_HTTP_HTTPREQUEST_HPP
