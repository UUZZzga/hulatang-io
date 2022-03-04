#ifndef HULATANG_BASE_FILE_HPP
#define HULATANG_BASE_FILE_HPP

#include "hulatang/base/Buf.hpp"
#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>

namespace hulatang::base {
enum OFlag
{
    O_READ = 1,
    O_WRITE = 2,
    O_EXEC = 4,
};

class FileDescriptor
{
public:
    FileDescriptor();
    ~FileDescriptor() noexcept;

    // open file
    void open(std::string_view path, OFlag oflag);

    // bind
    void bind(std::string_view localHost, int localPort);

    // connect to remote
    void connect(std::string_view peerHost, int peerPort) noexcept;

    void read(const Buf &buf, std::error_condition &ec) noexcept;

    void write(const Buf &buf, std::error_condition &ec) noexcept;

    void close() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

enum class FileErrorCode : int
{
    NOT_FOUND,
    ALREADY_EXISTS,
    PERMISSION_DENIED,
    RESOURCE_EXHAUSTED,
    FAILED_PRECONDITION
};

class file_error_category : public std::error_category
{
public:
    // 66696C65 ASCII is file
    constexpr static uintptr_t FILE_ERROR_ADDR = 0x66696C65;
    constexpr file_error_category() noexcept
        : error_category(FILE_ERROR_ADDR)
    {}

    [[nodiscard]] const char *name() const noexcept override
    {
        return "file_error";
    }

    [[nodiscard]] std::string message(int error_code) const override
    {
        static constexpr std::string_view unknown_error = "unknown error";
        return std::string(unknown_error);
    }

    [[nodiscard]] std::error_condition default_error_condition(int error_code) const noexcept override;
};

[[nodiscard]] inline const std::error_category &file_category() noexcept
{
    static file_error_category category;
    return category;
}

[[nodiscard]] inline std::error_condition make_file_error_condition(FileErrorCode error_code) noexcept
{
    return {static_cast<int>(error_code), file_category()};
}

} // namespace hulatang::base

#endif // HULATANG_BASE_FILE_HPP