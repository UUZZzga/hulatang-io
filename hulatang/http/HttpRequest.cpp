#include "hulatang/http/HttpRequest.hpp"

#include <magic_enum.hpp>

#include <algorithm>
#include <memory>

namespace hulatang::http {
HttpRequest::HttpRequest()
    : method_(INVALID)
    , version_(Version::Unknown)
    , headers_(std::make_unique<HeaderMap>())
{}

void HttpRequest::addHeader(std::string_view name, std::string_view value)
{
    (*headers_)[std::string{name}] = value;
}

void HttpRequest::addHeader(const char *start, const char *colon, const char *end)
{
    auto isSpace = [](char c) { return (isspace(c) != 0); };
    const auto *begin = std::find_if_not(colon + 1, end, isSpace);

    std::string_view value(begin, end);
    auto end_ = std::find_if_not(rbegin(value), rend(value), isSpace);

    addHeader({start, colon}, {begin, &*end_});
}

std::string HttpRequest::getHeader(const std::string &field) const
{
    std::string result;
    auto pos = headers_->find(field);
    if (pos != headers_->end())
    {
        result = pos->second;
    }
    return result;
}

void HttpRequest::swap(HttpRequest &that) noexcept
{
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    std::swap(receiveTime_, that.receiveTime_);
    headers_.swap(that.headers_);
    path_.swap(that.path_);
    query_.swap(that.query_);
}

std::string_view HttpRequest::versionString() const
{
    return to_string(version_);
}

bool HttpRequest::setVersionFromString(std::string_view version)
{
    version_ = VersionFromString(version);
    return version_ != Version::Unknown;
}

std::string_view HttpRequest::methodString() const
{
    return magic_enum::enum_name(method_);
}

bool HttpRequest::setMethodFromString(std::string_view method)
{
    if (method == "GET")
    {
        method_ = GET;
    }
    else if (method == "POST")
    {
        method_ = POST;
    }
    else if (method == "HEAD")
    {
        method_ = HEAD;
    }
    else if (method == "PUT")
    {
        method_ = PUT;
    }
    else if (method == "DELETE")
    {
        method_ = DELETE;
    }
    else
    {
        method_ = INVALID;
    }
    return method_ != INVALID;
}

void HttpRequest::setPath(std::string_view path)
{
    path_ = path;
}

std::ostream &operator<<(std::ostream &out, const HttpRequest &request)
{
    out << magic_enum::enum_name(request.method()) << " " << request.path() << " " << request.versionString() << "\r\n";
    for (const auto &header : request.headers())
    {
        out << header.first << ":" << header.second << "\r\n";
    }
    out << "\r\n";
    // TODO body
    return out;
}
} // namespace hulatang::http
