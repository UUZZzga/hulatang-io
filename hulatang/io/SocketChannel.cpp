#include "hulatang/io/SocketChannel.hpp"

#include "hulatang/base/Buf.hpp"

namespace hulatang::io {

SocketChannel::SocketChannel(EventLoop *loop, base::FileDescriptor &fd)
    : Channel(loop, fd)
    , state(kConnecting)
    , triggerByteNum(0)
    , waitSendByteNum(0)
    , waitSendCacheByteNum(0)
    , finishedSendByteNum(0)
{}

SocketChannel::~SocketChannel()
{
    if (state != kDisconnected)
    {
        forceCloseInLoop();
    }
}

void SocketChannel::close()
{
    if (state == kConnected || state == kDisconnecting)
    {
        state = kDisconnecting;

        if (loop->isInLoopThread())
        {
            forceCloseInLoop();
        }
        else
        {
            loop->queueInLoop([_this = shared_from_this()] { _this->forceCloseInLoop(); });
        }
    }
}

void SocketChannel::connectFailed()
{
    loop->assertInLoopThread();
    assert(state == kConnecting);
    Channel::disableAll();
    setState(kDisconnected);
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

    bool faultError = false;
    if (state == kDisconnected)
    {
        HLT_CORE_WARN("disconnected, give up writing");
        return;
    }

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
    // EventLoop 队列里存在事件但还没执行 forceCloseInLoop 被自己线程 再次执行
    if (state == kDisconnecting)
    {
        auto ptr = watcher.lock();
        if (!ptr)
        {
            return;
        }
        loop->getFdEventManager().cancel(ptr);
        fd.close();
        state = kDisconnected;
        if (connectionCallback)
        {
            connectionCallback(shared_from_this());
        }
    }
}

void SocketChannel::runSend(const base::Buf &buf)
{
    if (state == kConnected)
    {
        loop->runInLoop([_this = shared_from_this(), buf] { _this->sendInLoop(buf); });
    }
    DLOG_TRACE;
}

void SocketChannel::sendByteNum(size_t num)
{
    loop->assertInLoopThread();

    finishedSendByteNum += num;
    bool disableWriting = false;
    {
        std::lock_guard lock(writeLock);
        if (finishedSendByteNum == waitSendCacheByteNum)
        {
            finishedSendByteNum = 0;
            waitSendCacheByteNum = 0;
            disableWriting = true;
        }
    }
    if (disableWriting) {}
}
void SocketChannel::recvByteNum(size_t num)
{
    if (num > 0)
    {
        messageCallback(shared_from_this());
    }
    else if (num == 0)
    {
        close();
    }
}

void SocketChannel::update(int oldflag)
{
#if _WIN32
    if (oldflag == 0)
    {
        std::error_condition condition;
        fd.read({buffer.get(), bufferSize}, condition);
    }
#endif
}

void SocketChannel::handleRead() {}

} // namespace hulatang::io