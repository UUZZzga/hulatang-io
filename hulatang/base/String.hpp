#ifndef HULATANG_BASE_STRING_HPP
#define HULATANG_BASE_STRING_HPP

#include <sstream>

namespace hulatang::base {
std::string to_string(auto const& value)
{
    std::stringstream sstream;
    sstream << value;
    return sstream.str();
}
} // namespace hulatang::base

#endif // HULATANG_BASE_STRING_HPP
