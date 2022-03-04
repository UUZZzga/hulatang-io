#include "hulatang/io/async/IOCPFdEventManager.hpp"

#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/SocketChannel.hpp"
#include <chrono>

constexpr uintptr_t WAKEUP_NUMBER = 1;

namespace hulatang::io {
IOCPFdEventManager::IOCPFdEventManager()
    : iocpHandle(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
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

        Channel *channel = (Channel *)ptr;

        if (bRet == false)
        {
            int err = WSAGetLastError();
            if (err == WAIT_TIMEOUT || err == ERROR_OPERATION_ABORTED)
            {
                break;
            }
            if (ERROR_NETNAME_DELETED == err && channel)
            {
                // 远程主机断开连接
                channel->close();
                continue;
            }
            if (ERROR_CONNECTION_REFUSED == err)
            {
                if (channel)
                {
                    SocketChannel *socket_channel = dynamic_cast<SocketChannel *>(channel);
                    socket_channel->connectFailed();
                    continue;
                }
            }
            // LOG_SYSFATAL;
            // return -err;
        }

        SocketChannel *socket_channel = dynamic_cast<SocketChannel *>(channel);
        // sockets::PRE_IO_DATA *data = CONTAINING_RECORD(pOverlapped, sockets::PRE_IO_DATA, overlapped);
        // int type = data->operationType;
        // data->operationType = -1;
        // if (type == 0)
        // {
        //     assert(socket_channel->isReading());
        //     socket_channel->recvByteNum(bytes);
        //     channel->handleEvent();
        // }
        // else if (type == 1)
        // {
        //     assert(socket_channel->isWriting());
        //     socket_channel->sendByteNum(bytes);
        // }
        // else if (type == 2)
        // {
        //     // accept
        //     channel->handleRead();
        // }
        // else if (type == 3)
        // {
        //     // connect
        //     if (socket_channel->isConnecting())
        //     {
        //         socket_channel->connectEstablished();
        //     }
        //     else
        //     {
        //         channel->close();
        //     }
        // }

        loop->updateTime();
    }
}
} // namespace hulatang::io