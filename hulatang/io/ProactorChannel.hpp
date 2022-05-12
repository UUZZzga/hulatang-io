#ifndef HULATANG_IO_ProactorChannel_HPP
#define HULATANG_IO_ProactorChannel_HPP

#include "hulatang/base/Buf.hpp"
#include "hulatang/base/Buffer.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/FdEventWatcher.hpp"

#include <atomic>
#include <cassert>
#include <deque>
#include <memory>
#include <mutex>

namespace hulatang::io {
class TCPConnection;
class ProactorChannel : public std::enable_shared_from_this<ProactorChannel>, public Channel
{
public:
    typedef std::function<void(const base::Buf &)> MessageCallback;

public:
    ProactorChannel(EventLoop *loop, base::FileDescriptor &&fd, FdEventWatcherPtr _watcher, std::shared_ptr<void> _tie);

    ~ProactorChannel() override;

    void close() override;

public:
    // shutdownInput();
    // shutdownOutput();

    void connectFailed();

public:
    void handleRead(const base::Buf &buf);

    void handleWrite(const base::Buf &buf);

    void handleError() {}

    bool send(const base::Buf &buf);

    void setConnectionCallback(const DefaultCallback &connectionCallback);

    void setNextTriggerByteNum(size_t triggerByteNum);

    void setMessageCallback(const MessageCallback &messageCallback);

    void sendByteNum(char *buf, size_t num);

    void recvByteNum(char *buf, size_t num);

    void update(int oldflag) override;

    void handleRead() override;

    void postRead();
    void postWrite();

private:
    void forceCloseInLoop();

    void runSend(const base::Buf &buf);

private:
    DefaultCallback connectionCallback;
    MessageCallback messageCallback;
    std::weak_ptr<FdEventWatcher> watcher;
    std::shared_ptr<void> tie;
    size_t triggerByteNum;
    std::mutex writeLock;
    size_t finishedSendByteNum;
    bool closed;
};

inline void ProactorChannel::setConnectionCallback(const DefaultCallback &connectionCallback)
{
    this->connectionCallback = connectionCallback;
}

inline void ProactorChannel::setNextTriggerByteNum(size_t triggerByteNum)
{
    this->triggerByteNum = triggerByteNum;
}

inline void ProactorChannel::setMessageCallback(const MessageCallback &messageCallback)
{
    this->messageCallback = messageCallback;
}

} // namespace hulatang::io

#endif // HULATANG_IO_ProactorChannel_HPP
