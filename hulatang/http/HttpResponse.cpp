#include "hulatang/http/HttpResponse.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/extend/Defer.hpp"
#include "hulatang/http/HttpConstant.hpp"
#include "magic_enum.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>

namespace {
struct HttpResponseBuilder
{
    enum ErrorCode
    {
        NoError,
        Status,
        Messages,
        Body,
    };
    hulatang::http::HttpResponse::HeaderMap headers;
    std::string body;
    hulatang::base::Buffer *buf{};
    size_t readNumber{};
    const char *crlf{};
    const char *data{};
    hulatang::http::Version version{};
    hulatang::http::StatusCode status;
    std::string_view statusMessage;
    std::string_view line;
    ErrorCode code{};

    void nextLine()
    {
        readNumber += crlf - data + 2;
        data = crlf + 2;
        crlf = buf->findCRLF(data);
    }

    void statusLine()
    {
        // 状态行
        code = Status;
        line = {data, crlf};
        size_t i = 0;
        auto get = [&]() {
            i = line.find(' ');
            defer[&]
            {
                line = line.substr(i + 1);
            };
            return line.substr(0, i);
        };
        version = hulatang::http::VersionFromString(get());
        status = static_cast<hulatang::http::StatusCode>(std::stoi(std::string{get()}));
        statusMessage = get();
        code = NoError;
    }

    void messageLine()
    {
        // 消息报头
        code = Messages;
        while (true)
        {
            nextLine();
            line = {data, crlf};
            if (line.empty())
            {
                break;
            }
            auto i = line.find(':');
            std::string_view name = line.substr(0, i);
            std::string_view value = line.substr(i + 1);
            if (value.starts_with(' '))
            {
                value = value.substr(1);
            }
            headers.emplace(name, value);
        }
        code = NoError;
    }

    void bodyLine()
    {
        // 正文
        code = Body;
        auto pos = headers.find("Transfer-Encoding");
        if (pos != headers.end() && pos->second == "chunked")
        {
            transferEncoding();
            return;
        }
        if (pos = headers.find("Content-Length"); pos != headers.end())
        {
            contentLength(std::stoul(pos->second));
            return;
        }
        // TODO error code
    }

    void transferEncoding()
    {
        while (true)
        {
            nextLine();
            if (crlf == nullptr)
            {
                // 数据还没接收完
                return;
            }
            line = {data, crlf};
            nextLine();
            if (crlf == nullptr)
            {
                // 数据还没接收完
                return;
            }
            size_t len = std::stoul(std::string{line}, nullptr, 16);
            if (len == 0)
            {
                // 长度为0，则没有后续数据
                nextLine();
                if (crlf == nullptr)
                {
                    // 数据还没接收完
                    return;
                }
                buf->retrieve(readNumber);
                break;
            }
            if (buf->readableBytes() < readNumber + len + 2)
            {
                return;
            }
            // FIXME: 可以优化
            body += std::string_view{buf->data() + readNumber, len};
            readNumber += len;
            data = buf->data() + readNumber;
        }
        code = NoError;
    }

    void contentLength(size_t len)
    {
        // TODO 还未实现
        if (buf->readableBytes() < readNumber + len + 2)
        {
            // 还没读完
            return;
        }
        buf->retrieve(readNumber + 2);
        body = buf->retrieveAsString(len);
        code = NoError;
        HLT_CORE_TRACE("{}", body);
    }

    void parse()
    {
        data = buf->data();
        crlf = buf->findCRLF();
        statusLine();
        if (code != NoError)
        {
            return;
        }
        messageLine();
        if (code != NoError)
        {
            return;
        }
        bodyLine();
    }
};

class HttpResponseParams : public std::error_category
{
public:
    constexpr static uintptr_t ERROR_ADDR = 0xfa1a35;
    constexpr HttpResponseParams() noexcept
        : error_category(ERROR_ADDR)
    {}

    [[nodiscard]] const char *name() const noexcept override
    {
        return "HttpResponseParams";
    }
    [[nodiscard]] std::string message(int _Errval) const override
    {
        auto name = magic_enum::enum_name(static_cast<HttpResponseBuilder::ErrorCode>(_Errval));
        return std::string{name};
    }
    static const HttpResponseParams &getInstance()
    {
        static HttpResponseParams instance;
        return instance;
    }
};

} // namespace

namespace hulatang::http {

HttpResponse::HttpResponse()
    : statusCode_()
    , version_()
    , closeConnection_(false)
{}

HttpResponse::HttpResponse(HttpResponse &&other) noexcept
{
    HttpResponse response;
    response.swap(other);
    swap(response);
}

HttpResponse HttpResponse::buildFromBuffer(base::Buffer &buf, std::error_condition &condition)
{
    HttpResponseBuilder builder;
    builder.buf = &buf;

    builder.parse();
    if (builder.code == HttpResponseBuilder::NoError)
    {
        return {std::move(builder.headers), std::move(builder.body), builder.status, builder.version};
    }
    condition.assign(static_cast<int>(builder.code), HttpResponseParams::getInstance());
    return {};
}

void HttpResponse::addHeaders(std::string_view name, std::string_view value)
{
    if (!headers_)
    {
        headers_ = std::make_unique<HeaderMap>();
    }
    headers_->emplace(name, value);
}

std::string HttpResponse::getHeader(const std::string &name) const
{
    std::string result;
    auto pos = headers_->find(name);
    if (pos != headers_->end())
    {
        result = pos->second;
    }
    return result;
}

void HttpResponse::swap(HttpResponse &other) noexcept
{
    headers_.swap(other.headers_);
    body_.swap(other.body_);
    std::swap(statusCode_, other.statusCode_);
    std::swap(version_, other.version_);
    std::swap(closeConnection_, other.closeConnection_);
}

HttpResponse::HttpResponse(HeaderMap &&headers, std::string &&body, StatusCode status, Version version)
    : headers_(std::make_unique<HeaderMap>(move(headers)))
    , body_(move(body))
    , statusCode_(status)
    , version_(version)
    , closeConnection_(false)
{}
} // namespace hulatang::http