#ifndef HULATANG_BASE_EXCEPTION_HPP
#define HULATANG_BASE_EXCEPTION_HPP

#include <string>
#include <exception>

namespace hulatang {
namespace base {
class Exception : public std::exception
{
public:
    explicit Exception(std::string what);
    ~Exception() noexcept override = default;

    const char *what() const noexcept override
    {
        return message.c_str();
    }

    const char *stackTrace() const noexcept
    {
        return stack.c_str();
    }

private:
    std::string message;
    std::string stack;
};
} // namespace base

using base::Exception;

} // namespace hulatang

#endif // HULATANG_BASE_EXCEPTION_HPP
