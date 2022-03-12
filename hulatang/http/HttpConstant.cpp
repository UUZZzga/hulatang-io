#include "hulatang/http/HttpConstant.hpp"

#include <magic_enum.hpp>
#include <stdexcept>

namespace hulatang::http {
Version VersionFromString(std::string_view ver)
{
    if (ver.starts_with("HTTP/1.") && ver.size() == 8)
    {
        if (ver[7] == '0')
        {
            return Version::Http10;
        }
        if (ver[7] == '1')
        {
            return Version::Http11;
        }
    }
    return Version::Unknown;
}

StatusCode StatusFromString(std::string_view status)
{
    auto e = magic_enum::enum_cast<StatusCode>(status);
    if (!e.has_value())
    {
        throw std::runtime_error("Unknown status code");
    }
    return e.value();
}

std::string_view to_string(hulatang::http::Version version)
{
    switch (version)
    {
    case Version::Http10:
        return "HTTP/1.0";
    case Version::Http11:
        return "HTTP/1.1";
    default:
        return "Unknown";
    }
}
} // namespace hulatang::http