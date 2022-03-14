#ifndef HULATANG_BASE_FILE_HPP
#define HULATANG_BASE_FILE_HPP

#include "hulatang/base/def.h"
#include "hulatang/base/Buf.hpp"
#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>

struct sockaddr;

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
    class ErrorCategory;

    FileDescriptor();
    ~FileDescriptor() noexcept;

    FileDescriptor(FileDescriptor &&other) noexcept;

    [[nodiscard]] uintptr_t getFd() const noexcept;

    // open
    void open(std::string_view path, OFlag oflag, std::error_condition &condition);

    // create
    void create(std::string_view path, OFlag oflag, std::error_condition &condition);

    // lseek
    int64_t lseek(int64_t offset, int whence, std::error_condition &condition);

#if defined(HLT_PLATFORM_WINDOWS)
    void updatePosition(uint64_t position);
#endif

    // ======================= socket ==================================
    // bind
    void bind(std::string_view localHost, int localPort);

    // listen
    void listen(std::error_condition &condition);

    // accept
    void accept(FileDescriptor &fd, std::error_condition &condition);

    // connect to remote
    void connect(sockaddr *addr, size_t len, std::error_condition &condition) noexcept;

    void read(const Buf &buf, std::error_condition &condition) noexcept;

    void write(const Buf &buf, std::error_condition &condition) noexcept;

    void close() noexcept;

    const sockaddr *peeraddr();
    size_t peeraddrLength();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

enum FileErrorCode : int
{
    NONE,
    CONNECTING,
    _ERROR,
    NOT_FOUND = _ERROR,
    DEADLINE_EXCEEDED,
    OUT_OF_RANGE,
    ALREADY_EXISTS,
    PERMISSION_DENIED,
    RESOURCE_EXHAUSTED,
    FAILED_PRECONDITION,
    CONNECTION_REFUSED,
};

class FileDescriptor::ErrorCategory : public std::error_category
{
public:
    // 66696C65 ASCII is file
    constexpr static uintptr_t FILE_ERROR_ADDR = 0x66696C65;
    constexpr ErrorCategory() noexcept
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

    [[nodiscard]] std::error_condition default_error_condition(int error_code) const noexcept override
    {
        return {error_code, *this};
    }
};

[[nodiscard]] inline const std::error_category &file_category() noexcept
{
    static FileDescriptor::ErrorCategory category;
    return category;
}

[[nodiscard]] inline std::error_condition make_file_error_condition(FileErrorCode error_code) noexcept
{
    return {static_cast<int>(error_code), file_category()};
}

} // namespace hulatang::base

#endif // HULATANG_BASE_FILE_HPP
