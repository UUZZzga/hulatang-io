#ifndef HULATANG_HTTP_HTTPRESPONSE_HPP
#define HULATANG_HTTP_HTTPRESPONSE_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/http/HttpConstant.hpp"

#include <memory>
#include <string_view>
#include <system_error>
#include <unordered_map>

namespace hulatang::http {
class HttpResponse
{
public:
    using HeaderMap = std::unordered_map<std::string, std::string>;

    HttpResponse();
    HttpResponse(HttpResponse &&other) noexcept;
    HttpResponse(const HttpResponse &other);

    static HttpResponse buildFromBuffer(base::Buffer &buf, std::error_condition &condition);

    void addHeaders(std::string_view name, std::string_view value);

    void setBody(std::string_view body)
    {
        body_ = body;
    }
    void setBody(std::string &&body)
    {
        body_ = move(body);
    }

    [[nodiscard]] const std::string &body() const
    {
        return body_;
    }

    void setStatus(std::string_view status);

    void setVersion(const http::Version &version)
    {
        version_ = version;
    }

    [[nodiscard]] std::string getHeader(const std::string &name) const;

    void swap(HttpResponse &other) noexcept;

private:
    HttpResponse(HeaderMap &&headers, std::string &&body, StatusCode status, Version version);

    std::unique_ptr<HeaderMap> headers_;
    std::string body_;
    // std::string statusMessage_;
    StatusCode statusCode_;
    Version version_;
    bool closeConnection_;
};
} // namespace hulatang::http

#endif // HULATANG_HTTP_HTTPRESPONSE_HPP
