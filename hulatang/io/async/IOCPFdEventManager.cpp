#include "hulatang/io/async/IOCPFdEventManager.hpp"

#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/SocketChannel.hpp"
#include <chrono>

constexpr uintptr_t WAKEUP_NUMBER = 1;

namespace hulatang::io {
IOCPFdEventManager::IOCPFdEventManager()
    : iocpHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0))
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
                auto ec = make_win32_error_code(WSAGetLastError());
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
                watcher->closeHandle();
            }
            continue;
        }
        auto *data = CONTAINING_RECORD(pOverlapped, base::IO_DATA, overlapped);
        int type = data->operationType;
        data->operationType = -1;
        if (type == 0)
        {
            watcher->readHandle(bytes);
        }
        else if (type == 1)
        {
            watcher->writeHandle(bytes);
        }
        else if (type == 2)
        {
            // accept
        }
        else if (type == 3)
        {
            // connect
            watcher->openHandle();
        }
        else if (type == 4)
        {
            watcher->closeHandle();
        }

        loop->updateTime();
    }
}

void IOCPFdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &descriptor)
{
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(descriptor.getFd()), iocpHandle, reinterpret_cast<ULONG_PTR>(watcher.get()), 0);
}

void IOCPFdEventManager::cancel(const FdEventWatcherPtr &watcher)
{
    // if (m_io_datas[0].operationType != -1)
    // {
    //     CancelIoEx((HANDLE)getFd(), &(m_io_datas[0].overlapped));
    //     m_io_datas[0].operationType = -1;
    // }
    // if (m_io_datas[1].operationType != -1)
    // {
    //     CancelIoEx((HANDLE)getFd(), &(m_io_datas[1].overlapped));
    //     m_io_datas[1].operationType = -1;
    // }
    // disableAll();
}
} // namespace hulatang::io