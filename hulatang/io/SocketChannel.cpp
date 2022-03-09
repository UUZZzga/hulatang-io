#include "hulatang/io/SocketChannel.hpp"

#include "hulatang/base/Buf.hpp"

namespace hulatang::io {

SocketChannel::SocketChannel(EventLoop *loop, base::FileDescriptor &fd, FdEventWatcherPtr _watcher)
    : Channel(loop, fd)
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
    if (isWriting() || waitSendByteNum == 0)
    {
        return;
    }
    // disableWriting in down
    enableWriting();

    std::error_condition ec;
    fd.write(buf, ec);
}

void SocketChannel::forceCloseInLoop()
{
    loop->assertInLoopThread();
    auto ptr = watcher.lock();
    assert(ptr);
    connectionCallback(shared_from_this());
    loop->getFdEventManager().cancel(ptr);
    fd.close();
}

void SocketChannel::runSend(const base::Buf &buf)
{
    loop->runInLoop([_this = shared_from_this(), buf] { _this->sendInLoop(buf); });
    DLOG_TRACE;
}

void SocketChannel::sendByteNum(size_t num)
{
    loop->assertInLoopThread();

    // finishedSendByteNum += num;
    // bool disableWriting = false;
    // {
    //     std::lock_guard lock(writeLock);
    //     if (finishedSendByteNum == waitSendCacheByteNum)
    //     {
    //         finishedSendByteNum = 0;
    //         waitSendCacheByteNum = 0;
    //         disableWriting = true;
    //     }
    // }
    // if (disableWriting) {}
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
#if _WIN32
    if (oldflag == 0 && (flags & kReadEvent) != 0)
    {
        std::error_condition condition;
        fd.read({buffer.get(), bufferSize}, condition);
    }
#endif
}

void SocketChannel::handleRead() {}

} // namespace hulatang::io