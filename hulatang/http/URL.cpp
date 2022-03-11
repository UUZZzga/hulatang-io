#include "hulatang/http/URL.hpp"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <corecrt_math.h>
#include <iterator>
#include <stdexcept>
#include <string_view>

constexpr auto npos = std::string::npos;

namespace {
bool isHostnameChar(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || ('+' == c) || ('-' == c) || ('.' == c);
}

bool checkHostname(std::string_view v)
{
    return std::all_of(v.begin(), v.end(), isHostnameChar);
}

bool AllOfNumber(std::string_view v)
{
    return std::all_of(v.begin(), v.end(), [](char c) { return '0' <= c && c <= '9'; });
}

bool checkIP(std::string_view v)
{
    size_t v6start = v.find('[');
    size_t v6end = v.find_last_of(']');
    if (v6start != v6end)
    {
        if (v6start == npos || v6end == npos || v6start > v6end)
        {
            // 不合法的ipv6地址
            return false;
        }
    }
    return true;
}

bool tryPort(std::string_view url)
{
    // <ip/host>:<port>
    //          ^
    return !(url.empty() || !AllOfNumber(url));
}

bool tryHostPort(std::string_view url, size_t colonIndex)
{
    return tryPort(url.substr(colonIndex + 1)) && (checkHostname(url.substr(0, colonIndex)) || checkIP(url.substr(0, colonIndex)));
}
} // namespace

namespace hulatang::http {

URL::URL(std::string url)
    : url_(move(url))
{
    split();
}

void URL::split()
{
    std::string_view url{url_};
    auto i = url.find(':');
    if (tryHostPort(url, i))
    {
        return;
    }
    protocol_ = url.substr(0, i);
    url = url.substr(i + 1);
    if (url.starts_with("//"))
    {
        // <protocol>://<host>/<path>?<query>#<ref>
        //            ^
        size_t set[3] = {'/', '?', '#'};
        std::transform(std::begin(set), std::end(set), std::begin(set), [&](size_t c) { return url.find(static_cast<char>(c), 2); });
        auto *i = std::min_element(std::begin(set), std::end(set));
        auto hostport = url.substr(2, *i - 2);
        url = url.substr(*i);
        auto colonIndex = hostport.find_last_of(':');
        if (colonIndex != npos && !tryHostPort(hostport, colonIndex))
        {
            return;
        }
        host_ = hostport.substr(0, colonIndex);
        if (colonIndex != npos)
        {
            port_ = hostport.substr(colonIndex + 1);
        }
    }
    if (size_t i = url.find_last_of('#'); i != npos)
    {
        // <protocol>://<host>/<path>?<query>#<ref>
        //                                   ^
        ref_ = url.substr(i + 1);
        url = url.substr(0, i);
    }
    if (size_t i = url.find('?'); i != npos)
    {
        // <protocol>://<host>/<path>?<query>#<ref>
        //                           ^
        query_ = url.substr(i + 1);
        url = url.substr(0, i);
    }
    path_ = url;
}
} // namespace hulatang::http