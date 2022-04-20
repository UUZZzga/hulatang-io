#include "hulatang/io/SocketChannel.hpp"

#include "hulatang/base/Buf.hpp"

namespace hulatang::io {

SocketChannel::SocketChannel(EventLoop *loop, base::FileDescriptor &&fd, FdEventWatcherPtr _watcher, std::shared_ptr<void> _tie)
    : Channel(loop, std::move(fd))
    , watcher(_watcher)
    , tie(_tie)
    , triggerByteNum(0)
    , finishedSendByteNum(0)
    , closed(false)
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
    if (ec)
    {
        if (ec.value() == base::FileErrorCode::EEOF)
        {
            // �յ� FIN
            disableReading();
        }
        else if (ec.value() == base::FileErrorCode::RESET)
        {
            // ��Ϊ���� handleRead �� messageCallback
            // ���̹رպ� messageCallback ���޷�����д��
            // �Ӻ�close ��messageCallback��д�벻�����쳣
            loop->queueInLoop([_this = shared_from_this()] { _this->forceCloseInLoop(); });
        }
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
    sendBuffer.append(buf.buf, buf.len);
    enableWriting();
}

void SocketChannel::forceCloseInLoop()
{
    loop->assertInLoopThread();
    if (closed)
    {
        return;
    }
    closed = true;
    auto ptr = watcher.lock();
    assert(ptr);
    connectionCallback(shared_from_this());
    loop->getFdEventManager().cancel(ptr, fd);
    fd.close();
    tie.reset();
}

void SocketChannel::runSend(const base::Buf &buf)
{
    loop->runInLoop([_this = shared_from_this(), buf] { _this->sendInLoop(buf); });
    DLOG_TRACE;
}

void SocketChannel::sendByteNum(char *buf, size_t num)
{
    loop->assertInLoopThread();
    sendBuffer.retrieveAsString(num);
    if (sendBuffer.readableBytes() == 0)
    {
        disableWriting();
    }
    else
    {
        postWrite();
    }
}
void SocketChannel::recvByteNum(char *buf, size_t num)
{
    if (num == 0)
    {
        forceCloseInLoop();
        return;
    }
    handleRead(base::Buf{buf, num});
    messageCallback(base::Buf{buf, num});
}

void SocketChannel::update(int oldflag)
{
    if ((oldflag & kReadEvent) == 0 && (flags & kReadEvent) != 0)
    {
        postRead();
    }
    if ((oldflag & kWriteEvent) == 0 && (flags & kWriteEvent) != 0)
    {
        postWrite();
    }
    loop->getFdEventManager().change(fd);
}

void SocketChannel::handleRead() {}

void SocketChannel::postRead()
{
    std::error_condition condition;
    fd.read({buffer.get(), bufferSize}, condition);
}

void SocketChannel::postWrite()
{
    base::Buf buf(const_cast<char *>(sendBuffer.data()), sendBuffer.size());
    std::error_condition ec;
    fd.write(buf, ec);
    if (ec && ec.value() == base::FileErrorCode::RESET)
    {
        forceCloseInLoop();
    }
}
} // namespace hulatang::io