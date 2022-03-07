#ifndef CXX_PLATFORM_ERROR_WIN32_ERROR_CODE_HPP
#define CXX_PLATFORM_ERROR_WIN32_ERROR_CODE_HPP

#include <system_error>
#include <windows.h>

namespace hulatang {

class win32_error_category : public std::error_category
{ // categorize an operating system error
public:
    // 77696E00 ASCII is win
    constexpr static uintptr_t WIN32_ERROR_ADDR = 0x77696E00;
    constexpr win32_error_category() noexcept
        : error_category(WIN32_ERROR_ADDR)
    {}

    [[nodiscard]] const char *name() const noexcept override
    {
        return "win32_error";
    }

    [[nodiscard]] std::string message(int error_code) const override
    {
        char szBuf[128];
        DWORD result = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szBuf, sizeof(szBuf), nullptr);
        if (result == 0)
        {
            static constexpr std::string_view unknown_error = "unknown error";
            return std::string(unknown_error);
        }
        return std::string(szBuf, result);
    }

    [[nodiscard]] std::error_condition default_error_condition(int error_code) const noexcept override
    {
        return {error_code, *this};
    }
};

#if defined(_MSC_VER)
[[nodiscard]] inline const std::error_category &win32_category() noexcept
{
    static win32_error_category category;
    return category;
}
#endif

[[nodiscard]] inline std::system_error make_win32_system_error(int error_code) noexcept
{
    return {error_code, win32_category()};
}

[[nodiscard]] inline std::error_code make_win32_error_code(int error_code) noexcept
{
    return {error_code, win32_category()};
}
} // namespace hulatang

#endif // CXX_PLATFORM_ERROR_WIN32_ERROR_CODE_HPP
