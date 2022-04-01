#include "hulatang/io/SocketChannel.hpp"

#include "hulatang/base/Buf.hpp"

namespace hulatang::io {

SocketChannel::SocketChannel(EventLoop *loop, base::FileDescriptor &&fd, FdEventWatcherPtr _watcher)
    : Channel(loop, std::move(fd))
    , watcher(_watcher)
    , triggerByteNum(0)
    , waitSendByteNum(0)
    , waitSendCacheByteNum(0)
    , finishedSendByteNum(0)
{}

SocketChannel::~SocketChannel()
{
    forceCloseInLoop();
}

void SocketChannel::close()
{
    if (loop->isInLoopThread())
    {
        forceCloseInLoop();
    }
    else
    {
        loop->queueInLoop([_this = shared_from_this()] { _this->forceCloseInLoop(); });
    }
}

void SocketChannel::connectFailed()
{
    loop->assertInLoopThread();
    Channel::disableAll();
}

void SocketChannel::handleRead(const base::Buf &buf)
{
    loop->assertInLoopThread();

    if (!isReading())
    {
        enableReading();
    }
    std::error_condition ec;
    fd.read(buf, ec);
    if (ec && ec.value() == base::FileErrorCode::RESET)
    {
        forceCloseInLoop();
    }
}

void SocketChannel::handleWrite(const base::Buf &buf)
{
    runSend(buf);
}

bool SocketChannel::send(const base::Buf &buf)
{
    runSend(buf);
    return true;
}

void SocketChannel::sendInLoop(const base::Buf &buf)
{
    loop->assertInLoopThread();
    if (isWriting())
    {
        abort();
    }
    // disableWriting in down
    enableWriting();
    waitSendByteNum = buf.len;
    std::error_condition ec;
    fd.write(buf, ec);
    if (ec && ec.value() == base::FileErrorCode::RESET)
    {
        forceCloseInLoop();
    }
}

void SocketChannel::forceCloseInLoop()
{
    loop->assertInLoopThread();
    auto ptr = watcher.lock();
    assert(ptr);
    connectionCallback(shared_from_this());
    loop->getFdEventManager().cancel(ptr, fd);
    fd.close();
}

void SocketChannel::runSend(const base::Buf &buf)
{
    loop->runInLoop([_this = shared_from_this(), buf] { _this->sendInLoop(buf); });
    DLOG_TRACE;
}

void SocketChannel::sendByteNum(char *buf, size_t num)
{
    loop->assertInLoopThread();
    waitSendByteNum -= num;
    if (waitSendByteNum == 0)
    {
        disableWriting();
    }
    else
    {
        assert(HLT_PLATFORM_WINDOWS == 0);
        std::error_condition condition;
        fd.write({buf + num, waitSendByteNum}, condition);
    }
}
void SocketChannel::recvByteNum(char *buf, size_t num)
{
    if (num == 0)
    {
        forceCloseInLoop();
        return;
    }
    messageCallback(base::Buf{buf, num});
    handleRead(base::Buf{buf, num});
}

void SocketChannel::update(int oldflag)
{
    if ((oldflag & kReadEvent) == 0 && (flags & kReadEvent) != 0)
    {
        postRead();
        loop->getFdEventManager().add(watcher.lock(), fd);
    }
    if ((oldflag & kWriteEvent) != 0 || (flags & kWriteEvent) != 0)
    {
        std::error_condition condition;
        fd.write({}, condition);
        loop->getFdEventManager().add(watcher.lock(), fd);
    }
}

void SocketChannel::handleRead() {}

void SocketChannel::postRead()
{
    std::error_condition condition;
    fd.read({buffer.get(), bufferSize}, condition);
}

} // namespace hulatang::io