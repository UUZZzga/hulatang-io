#include "hulatang/io/ProactorChannel.hpp"

#include "hulatang/base/Buf.hpp"

namespace hulatang::io {

ProactorChannel::ProactorChannel(EventLoop *loop, base::FileDescriptor &&fd, FdEventWatcherPtr _watcher, std::shared_ptr<void> _tie)
    : Channel(loop, std::move(fd))
    , watcher(_watcher)
    , tie(_tie)
    , triggerByteNum(0)
    , finishedSendByteNum(0)
    , closed(false)
{}

ProactorChannel::~ProactorChannel()
{
    forceCloseInLoop();
}

void ProactorChannel::close()
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

void ProactorChannel::connectFailed()
{
    loop->assertInLoopThread();
    Channel::disableAll();
}

void ProactorChannel::handleRead(const base::Buf &buf)
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

void ProactorChannel::handleWrite(const base::Buf &buf)
{
    runSend(buf);
}

bool ProactorChannel::send(const base::Buf &buf)
{
    runSend(buf);
    return true;
}

void ProactorChannel::forceCloseInLoop()
{
    loop->assertInLoopThread();
    DLOG_TRACE;
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

void ProactorChannel::runSend(const base::Buf &buf)
{
    loop->runInLoop([_this = shared_from_this(), buf] { _this->sendInLoop(buf); });
    DLOG_TRACE;
}

void ProactorChannel::sendByteNum(char *buf, size_t num)
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
void ProactorChannel::recvByteNum(char *buf, size_t num)
{
    if (num == 0)
    {
        forceCloseInLoop();
        return;
    }
    handleRead(base::Buf{buf, num});
    messageCallback(base::Buf{buf, num});
}

void ProactorChannel::update(int oldflag)
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

void ProactorChannel::handleRead() {}

void ProactorChannel::postRead()
{
    std::error_condition condition;
    fd.read({buffer.get(), bufferSize}, condition);
}

void ProactorChannel::postWrite()
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