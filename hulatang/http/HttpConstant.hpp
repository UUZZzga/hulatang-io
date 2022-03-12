#ifndef HULATANG_HTTP_HTTPCONSTANT_HPP
#define HULATANG_HTTP_HTTPCONSTANT_HPP

#include <string>
#include <string_view>

namespace hulatang::http {
enum class Version
{
    Unknown,
    Http10,
    Http11
};

enum class StatusCode
{
    Continue = 100,
    SwitchingProtocols = 101,

    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,

    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    TemporaryRedirect = 307,

    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    RequestEntityTooLarge = 413,
    RequestURITooLarge = 414,
    UnsupportedMediaType = 415,
    RequestedRangeNotSatisfiable = 416,
    ExpectationFailed = 417,

    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505
};

Version VersionFromString(std::string_view ver);
StatusCode StatusFromString(std::string_view status);

std::string_view to_string(hulatang::http::Version);
} // namespace hulatang::http
#endif // HULATANG_HTTP_HTTPCONSTANT_HPP
