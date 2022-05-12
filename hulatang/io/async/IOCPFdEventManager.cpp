#include "hulatang/io/async/IOCPFdEventManager.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/extend/Cast.hpp"
#include "hulatang/base/File.hpp"
#include "hulatang/base/error/ErrorCode.hpp"
#include "hulatang/base/platform/win32/Type.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/Proactor.hpp"
#include "hulatang/io/proactor/WindowsOver.hpp"
#include <chrono>
#include <cstddef>
#include <stdlib.h>

constexpr uintptr_t WAKEUP_NUMBER = -1;

namespace hulatang::io {
static void freeOverList(Proactor::Over *over)
{
    if (over == nullptr)
    {
        return;
    }
    if (over->next == nullptr)
    {
        delete over;
        return;
    }
    freeOverList(over->next);
}

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
    , completionPortEntries(128)
{}

IOCPFdEventManager::~IOCPFdEventManager()
{
    CloseHandle(iocpHandle);
}

void IOCPFdEventManager::process(microseconds blockTime)
{
    microseconds currentTime = loop->getCurrentTime();
    microseconds timeout = loop->getCurrentTime() + blockTime;

    ULONG_PTR ptr = 0;
    DWORD bytes = 0;
    LPOVERLAPPED pOverlapped = nullptr;
    for (; currentTime < timeout; loop->updateTime(), currentTime = loop->getCurrentTime())
    {
        auto iocpBlockMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - currentTime).count();
        BOOL bRet = GetQueuedCompletionStatus(iocpHandle, &bytes, &ptr, &pOverlapped, implicit_cast<DWORD>(iocpBlockMs));
        // blockTime不为0的情况下
        // 比较使用前和使用后的时间（毫秒）没变化就继续执行

        if (ptr == WAKEUP_NUMBER)
        {
            return;
        }
        auto *channel = channels[ptr];
        if (bRet == FALSE)
        {
            int err = WSAGetLastError();
            if (err == WAIT_TIMEOUT || err == ERROR_OPERATION_ABORTED)
            {
                break;
            }
            if (channel == nullptr)
            {
                auto ec = make_win32_error_code(err);
                HLT_CORE_WARN("error code: {}, error message: {}", ec.value(), ec.message());
                continue;
            }
        }
        if (channel == nullptr)
        {
            return;
        }

        auto *overlapped = reinterpret_cast<WindowsOver *>(reinterpret_cast<char *>(pOverlapped) - WindowsOver::offset);
        overlapped->numberOfBytesTransferred = bytes;

        auto *over = reinterpret_cast<Proactor::Over *>(channel->getOver());
        if (over == overlapped)
        {
            channel->setOver(over->next);
            over->next = reinterpret_cast<WindowsOver *>(channel->getROver());
            channel->setROver(overlapped);
            channel->handler();
            continue;
        }
        auto *prevOver = over;
        over = over->next;
        while (over != nullptr)
        {
            assert(prevOver != nullptr);
            if (over == overlapped)
            {
                prevOver->next = over->next;
                over->next = reinterpret_cast<WindowsOver *>(channel->getROver());
                channel->setROver(overlapped);
                channel->handler();
                break;
            }
            prevOver = over;
            over = over->next;
        }
        if (over == nullptr)
        {
            abort();
        }
    }

// auto timeout = duration_cast<std::chrono::milliseconds>(blockTime);
// ULONG numEntriesRemoved;
// BOOL bRet = ::GetQueuedCompletionStatusEx(iocpHandle, completionPortEntries.data(), completionPortEntries.size(), &numEntriesRemoved,
//    implicit_cast<DWORD>(timeout.count()), FALSE);
// if (bRet == FALSE)
//{
//    auto err = WSAGetLastError();
//    if (err == WAIT_TIMEOUT)
//    {
//        return;
//    }
//    auto ec = make_win32_error_code(err);
//    HLT_CORE_WARN("error code: {}, error message: {}", ec.value(), ec.message());
//    abort();
//}
// for (auto i = 0; i < numEntriesRemoved; i++)
//{
//    auto &entry = completionPortEntries[i];
//    if (WAKEUP_NUMBER == entry.lpCompletionKey) {
//        continue;
//    }
//    auto fd = static_cast<base::fd_t>(entry.lpCompletionKey);
//    auto *channel = channels[fd];
//    if (channel == nullptr)
//    {
//        continue;
//    }
//    auto *overlapped = reinterpret_cast<WindowsOver *>(reinterpret_cast<char *>(entry.lpOverlapped) - WindowsOver::offset);
//    overlapped->numberOfBytesTransferred = entry.dwNumberOfBytesTransferred;
//    auto *over = reinterpret_cast<Proactor::Over *>(channel->getOver());
//    currentChannels.insert(channel);

//    if (over == overlapped)
//    {
//        channel->setOver(over->next);
//        over->next = reinterpret_cast<WindowsOver *>(channel->getROver());
//        channel->setROver(overlapped);
//        continue;
//    }
//    auto *prevOver = over;
//    over = over->next;
//    while (over != nullptr)
//    {
//        assert(prevOver != nullptr);
//        if (over == overlapped)
//        {
//            prevOver->next = over->next;
//            over->next = reinterpret_cast<WindowsOver *>(channel->getROver());
//            channel->setROver(overlapped);
//            break;
//        }
//        prevOver = over;
//        over = over->next;
//    }
//    if (over == nullptr)
//    {
//        abort();
//    }
//}
// for (auto *channel : currentChannels)
//{
//    channel->handler();
//}
// currentChannels.clear();
// if (numEntriesRemoved == completionPortEntries.size())
//{
//    completionPortEntries.resize(completionPortEntries.size() * 2);
//}
}

void IOCPFdEventManager::add(Channel *channel)
{
    FdEventManager::add(channel);
    ::CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(channel->getFd().getFd()), iocpHandle, static_cast<ULONG_PTR>(channel->getFd().getFd()), 0);
}

void IOCPFdEventManager::update(Channel *channel)
{
    Proactor::Over over{.next = static_cast<Proactor::Over *>(channel->getROver()), .type = Type::UPDATE};
    channel->setROver(&over);
    // handler 不释放 over
    channel->handler();
}

void IOCPFdEventManager::cancel(Channel *channel)
{
    // size_t readNum = 0;
    // bool write = false;
    // auto *over = reinterpret_cast<WindowsOver *>(channel->getOver());
    // while (over != nullptr)
    // {
    //     if (over->type == Type::READ)
    //     {
    //         ::CancelIoEx(reinterpret_cast<HANDLE>(channel->getFd().getFd()), &over->overlapped);
    //     }
    //     else
    //     {
    //         write = true;
    //     }
    //     over = static_cast<WindowsOver *>(over->next);
    // }
    // if (write)
    // {
    // }
    ::CancelIo(reinterpret_cast<HANDLE>(channel->getFd().getFd()));

    freeOverList(reinterpret_cast<WindowsOver *>(channel->getOver()));
    freeOverList(reinterpret_cast<WindowsOver *>(channel->getROver()));
    FdEventManager::cancel(channel);
}

void IOCPFdEventManager::wakeup()
{
    PostQueuedCompletionStatus(iocpHandle, WAKEUP_NUMBER, 0, nullptr);
}
} // namespace hulatang::io