#ifndef HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
#define HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP

#include "hulatang/base/File.hpp"

namespace hulatang::base {
struct FileDescriptor::Impl
{
    enum
    {
        READ,
        WRITE
    };
    // void *watcher{};
    // Buf readBuf;
    // Buf writeBuf;
    // int event{};
    int fd{};
    // bool accept : 1{};
    // bool connection : 1{};
    // bool read : 1{};
    // bool write : 1{};
    // bool recvfrom : 1{};
    // bool sendto : 1{};
    // bool close : 1{};
};
} // namespace hulatang::base

#endif // HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
