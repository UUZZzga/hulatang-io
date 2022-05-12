#include "hulatang/io/proactor/WSProactor.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include "hulatang/base/error/ErrorCode.hpp"

#include "hulatang/io/proactor/WindowsOver.hpp"

namespace {
void error(auto *channel, auto err)
{
    if (err != WSAECONNABORTED && err != WSAECONNRESET)
    {
        auto ec = hulatang::make_win32_error_code(err);
        channel->handleError(ec.default_error_condition());
    }
    channel->handleClose();
}
} // namespace

namespace hulatang::io {
constexpr size_t MinRecvBufferSize = 1024;

std::unique_ptr<Model> SocketModelFactory::create(
    Channel *channel, base::Buffer *recvBuf, base::Buffer *sendBuf, MessageCallback messageCallback, WriteCallback writeCallback)
{
    auto result = std::make_unique<WSProactor>(channel, recvBuf, sendBuf);
    result->setMessageCallback(messageCallback);
    result->setWriteCallback(writeCallback);
    return result;
}

struct StreamOverlapped : public WindowsOver
{
    char *data;
    size_t len;
};

WSProactor::WSProactor(Channel *channel, base::Buffer *recvBuf, base::Buffer *sendBuf)
    : Proactor()
    , readBuf(recvBuf)
    , writeBuf(sendBuf)
{
    channel->enableOver();
    postRecv(channel, new StreamOverlapped{});
}

void WSProactor::handleRead(Channel *channel, Over *over)
{
    auto *wsOver = static_cast<StreamOverlapped *>(over);

    auto writableBytes = readBuf->writableBytes();
    readBuf->hasWritten(wsOver->numberOfBytesTransferred);
    if (wsOver->numberOfBytesTransferred == 0)
    {
        // close
        channel->handleClose();
        return;
    }
    if (wsOver->numberOfBytesTransferred == writableBytes)
    {
        // 单个重叠io一次性读满，可能还有剩余数据
        // 补充一个无阻塞的读取
        char data[65536];
        WSABUF wsabuf{.len = sizeof(data), .buf = data};
        DWORD numberOfBytesRecvd = 0;
        DWORD flags = 0;
        if (SOCKET_ERROR == WSARecv(channel->getFd().getFd(), &wsabuf, 1, &numberOfBytesRecvd, &flags, nullptr, nullptr))
        {
            auto err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK)
            {
                error(channel, err);
                return;
            }
            // 避免重复触发 WSAEWOULDBLOCK
            readBuf->ensureWritableBytes(MinRecvBufferSize);
        }
        if (numberOfBytesRecvd > 0)
        {
            readBuf->append(data, numberOfBytesRecvd);
        }
    }
    messageCallback();

    // 重新添加一个读取
    postRecv(channel, wsOver);
}

void WSProactor::handleWrite(Channel *channel, Over *over)
{
    // TCPConnection::send -> Channel::enableWriting -> Channel::updata
    //   -> IOCPFdEventManager::cancel -> Proactor::handleEvent
    //   -> WSProactor::updatePost -> WSProactor::handleWrite
    auto *wsOver = static_cast<StreamOverlapped *>(over);

    writeBuf->retrieve(wsOver->len);

    if (writeCallback)
    {
        writeCallback(wsOver->data, wsOver->len);
    }

    if (writeBuf->readableBytes() > 0)
    {
        postSend(channel, wsOver);
    }
    else
    {
        // 没有需要发送的数据
        addFree(wsOver);
        channel->disableWriting();
    }
}

void WSProactor::updatePost(Channel *channel)
{
    if (channel->isWriting())
    {
        if (writeBuf->size() == 0)
        {
            return;
        }
        // 如果没有write的Over 则postSend一个
        auto *wsOver = static_cast<StreamOverlapped *>(channel->getOver());
        while (wsOver != nullptr)
        {
            if (wsOver->type == Type::WRITE && wsOver->data == writeBuf->data())
            {
                return;
            }
            wsOver = static_cast<StreamOverlapped *>(wsOver->next);
        }
        postSend(channel, new StreamOverlapped{});
    }
}

void WSProactor::postRecv(Channel *channel, Over *over)
{
    auto *wsOver = static_cast<StreamOverlapped *>(over);
    wsOver->type = Type::NONE;
    addOver(channel, wsOver);

    readBuf->ensureWritableBytes(MinRecvBufferSize);
    WSABUF wsabuf{.len = static_cast<ULONG>(readBuf->writableBytes()), .buf = readBuf->beginWrite()};
    DWORD flags = 0;

    if (SOCKET_ERROR == WSARecv(channel->getFd().getFd(), &wsabuf, 1, nullptr, &flags, &wsOver->overlapped, nullptr))
    {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            error(channel, err);
            return;
        }
    }
    wsOver->type = Type::READ;
    wsOver->overlapped = {};
    wsOver->numberOfBytesTransferred = 0;
    wsOver->data = wsabuf.buf;
    wsOver->len = wsabuf.len;
}

void WSProactor::postSend(Channel *channel, Over *over)
{
    assert(writeBuf->size() > 0);
    auto *wsOver = static_cast<StreamOverlapped *>(over);
    wsOver->type = Type::NONE;
    addOver(channel, wsOver);

    WSABUF wsabuf{.len = static_cast<ULONG>(writeBuf->size()), .buf = writeBuf->data()};
    DWORD flags = 0;

    if (SOCKET_ERROR == WSASend(channel->getFd().getFd(), &wsabuf, 1, nullptr, flags, &wsOver->overlapped, nullptr))
    {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING)
        {
            error(channel, err);
            return;
        }
    }

    wsOver->type = Type::WRITE;
    wsOver->data = wsabuf.buf;
    wsOver->len = wsabuf.len;
}

} // namespace hulatang::io
