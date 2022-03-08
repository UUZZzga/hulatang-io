#ifndef MYNET_TCP_CONNECTION_HPP_
#define MYNET_TCP_CONNECTION_HPP_

#include "hulatang/base/Buf.hpp"
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
class SocketChannel : public std::enable_shared_from_this<SocketChannel>, public Channel
{
public:
    typedef std::function<void(const base::Buf &)> MessageCallback;

public:
    SocketChannel(EventLoop *loop, base::FileDescriptor &fd, FdEventWatcherPtr _watcher);

    ~SocketChannel() override;

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

    void sendByteNum(size_t num);

    void recvByteNum(char *buf, size_t num);

    void update(int oldflag) override;

    void handleRead() override;

private:
    void sendInLoop(const base::Buf &buf);

    void forceCloseInLoop();

    void runSend(const base::Buf &buf);

private:
    DefaultCallback connectionCallback;
    MessageCallback messageCallback;

    std::weak_ptr<FdEventWatcher> watcher;
    size_t triggerByteNum;
    std::mutex writeLock;
    size_t waitSendByteNum;
    size_t waitSendCacheByteNum;
    size_t finishedSendByteNum;
};

inline void SocketChannel::setConnectionCallback(const DefaultCallback &connectionCallback)
{
    this->connectionCallback = connectionCallback;
}

inline void SocketChannel::setNextTriggerByteNum(size_t triggerByteNum)
{
    this->triggerByteNum = triggerByteNum;
}

inline void SocketChannel::setMessageCallback(const MessageCallback &messageCallback)
{
    this->messageCallback = messageCallback;
}

} // namespace hulatang::io
#endif // MYNET_TCP_CONNECTION_HPP_
