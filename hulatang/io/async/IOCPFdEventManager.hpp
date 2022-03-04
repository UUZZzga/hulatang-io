#ifndef HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
#define HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP

#include "hulatang/io/FdEventManager.hpp"

#include <Windows.h>

namespace hulatang::io {
class IOCPFdEventManager : public FdEventManager
{
public:
    IOCPFdEventManager();
    ~IOCPFdEventManager();

    void process(microseconds blockTime) override;

private:
    HANDLE iocpHandle;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ASYNC_IOCPFDEVENTMANAGER_HPP
