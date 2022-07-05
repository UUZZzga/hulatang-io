#include "hulatang/io/reactor/UnixReactor.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include <sys/uio.h>
#include <unistd.h>

namespace hulatang::io {
std::unique_ptr<Model> SocketModelFactory::create(
    Channel *channel, base::Buffer *recvBuf, base::Buffer *sendBuf, MessageCallback messageCallback, WriteCallback writeCallback)
{
    auto result = std::make_unique<UnixReactor>(recvBuf, sendBuf);
    result->setMessageCallback(messageCallback);
    result->setWriteCallback(writeCallback);
    return result;
}

UnixReactor::UnixReactor(base::Buffer *readBuf, base::Buffer *writeBuf)
    : readBuf(readBuf)
    , writeBuf(writeBuf)
{}

void UnixReactor::handleRead(Channel *channel)
{
    char extrabuf[65536];
    iovec iov[2];

    size_t writable = readBuf->writableBytes();

    iov[0] = {readBuf->beginWrite(), writable};
    iov[1] = {extrabuf, sizeof(extrabuf)};
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    auto read_num = readv(channel->getFd().getFd(), iov, iovcnt);

    if (read_num < 0)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            return;
        }
        if (err == ECONNRESET || err == EPIPE)
        {
            channel->handleClose();
        }
        else
        {
            auto ec = std::error_condition{err, std::system_category()};
            channel->handleError(ec);
            channel->handleClose();
        }
    }
    if (read_num == 0)
    {
        channel->handleClose();
        return;
    }
    if (read_num <= writable)
    {
        readBuf->hasWritten(read_num);
    }
    else
    {
        readBuf->hasWritten(writable);
        readBuf->append(extrabuf, read_num - writable);
    }
    messageCallback();
}

void UnixReactor::handleWrite(Channel *channel)
{
    auto num = ::write(channel->getFd().getFd(), writeBuf->data(), writeBuf->size());
    if (num < 0)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            return;
        }
        if (err == ECONNRESET || err == EPIPE)
        {
            channel->handleClose();
        }
        else
        {
            auto ec = std::error_condition{err, std::system_category()};
            channel->handleError(ec);
            channel->handleClose();
        }
    }
    else if (writeCallback)
    {
        writeCallback(writeBuf->data(), num);
    }
    writeBuf->retrieve(num);
}
} // namespace hulatang::io
