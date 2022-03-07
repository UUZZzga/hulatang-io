#include "hulatang/io/async/IOCPFdEventManager.hpp"

#include "hulatang/base/File.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/base/platform/win32/Type.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/SocketChannel.hpp"
#include <chrono>

constexpr uintptr_t WAKEUP_NUMBER = 1;

namespace hulatang::io {

std::unique_ptr<FdEventManager> FdEventManager::create(EventLoop *loop)
{
    return std::make_unique<IOCPFdEventManager>(loop);
}

using base::FileErrorCode;
using base::make_file_error_condition;
using base::Type::CLOSE;
using base::Type::NONE;
using base::Type::OPEN;
using base::Type::READ;
using base::Type::WRITE;

IOCPFdEventManager::IOCPFdEventManager(EventLoop *loop)
    : FdEventManager(loop)
    , iocpHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0))
{}

IOCPFdEventManager::~IOCPFdEventManager()
{
    CloseHandle(iocpHandle);
}

void IOCPFdEventManager::process(microseconds blockTime)
{
    microseconds currentTime = loop->getCurrentTime();
    microseconds timeout = currentTime + blockTime;
    int num = 0;
    ULONG_PTR ptr = 0;
    DWORD bytes = 0;
    LPOVERLAPPED pOverlapped = nullptr;
    for (; currentTime < timeout; ++num)
    {
        auto iocpBlockMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - currentTime).count();
        BOOL bRet = GetQueuedCompletionStatus(iocpHandle, &bytes, &ptr, &pOverlapped, iocpBlockMs);
        if (ptr == WAKEUP_NUMBER)
        {
            return;
        }

        auto *watcher = reinterpret_cast<FdEventWatcher *>(ptr);

        if (bRet == FALSE)
        {
            int err = WSAGetLastError();
            if (err == WAIT_TIMEOUT || err == ERROR_OPERATION_ABORTED)
            {
                break;
            }
            if (watcher == nullptr)
            {
                auto ec = make_win32_error_code(err);
                HLT_CORE_WARN("error code: {}, error message: {}", ec.value(), ec.message());
                continue;
            }
            if (ERROR_NETNAME_DELETED == err)
            {
                // 远程主机断开连接
                watcher->closeHandle();
            }
            else if (ERROR_CONNECTION_REFUSED == err)
            {
                // 远程计算机拒绝了网络连接。
                auto ec = make_file_error_condition(FileErrorCode::CONNECTION_REFUSED);
                watcher->errorHandle(ec);
            }
            continue;
        }
        auto *data = CONTAINING_RECORD(pOverlapped, base::IO_DATA, overlapped);
        auto type = data->operationType;
        data->operationType = base::Type::NONE;
        switch (type)
        {
        case OPEN: {
            // connect
            watcher->openHandle();
        }
        break;
        case CLOSE: {
            watcher->closeHandle();
        }
        break;
        case READ: {
            watcher->readHandle(bytes);
        }
        break;
        case WRITE: {
            watcher->writeHandle(bytes);
        }
        default: {
            HLT_CORE_ERROR("未知错误");
        }
        };
        loop->updateTime();
    }
}

void IOCPFdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &descriptor)
{
    FdEventManager::add(watcher, descriptor);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(descriptor.getFd()), iocpHandle, reinterpret_cast<ULONG_PTR>(watcher.get()), 0);
}

void IOCPFdEventManager::cancel(const FdEventWatcherPtr &watcher)
{
    FdEventManager::cancel(watcher);
    // disableAll();
}
} // namespace hulatang::io