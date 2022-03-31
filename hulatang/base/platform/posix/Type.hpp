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
    void *watcher;
    Buf readBuf;
    Buf writeBuf;
    int event;
    int fd;
    unsigned accept : 1;
    unsigned read : 1;
    unsigned write : 1;
    unsigned recvfrom : 1;
    unsigned sendto : 1;
    unsigned close : 1;
};
} // namespace hulatang::base

#endif // HULATANG_BASE_PLATFORM_WIN32_TYPE_HPP
