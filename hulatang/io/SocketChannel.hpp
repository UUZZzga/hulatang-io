#ifndef MYNET_TCP_CONNECTION_HPP_
#define MYNET_TCP_CONNECTION_HPP_

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
class SocketChannel : public std::enable_shared_from_this<SocketChannel>, public Channel
{
public:
    typedef std::function<void(const std::shared_ptr<SocketChannel> &)> DefaultCallback;
    typedef std::function<void(const std::shared_ptr<SocketChannel> &)> MessageCallback;

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

public:
    explicit SocketChannel(EventLoop *loop);

    ~SocketChannel() override;

    void close() override;

public:
    // shutdownInput();
    // shutdownOutput();

    void connectEstablished() override;

    void connectFailed();

public:
    void handleRead(const base::Buf &buf);

    void handleWrite(const base::Buf &buf);

    void handleError() {}

    bool send(const base::Buf &buf);

    bool isConnecting()
    {
        return state == kConnecting;
    }
    bool isConnected();

    void setConnectionCallback(const DefaultCallback &connectionCallback);

    void setNextTriggerByteNum(size_t triggerByteNum);

    void setMessageCallback(const MessageCallback &messageCallback);

    void sendByteNum(size_t num);

    void recvByteNum(size_t num);

protected:
    void setState(StateE state);

private:
    void sendInLoop(const base::Buf &buf);

    void forceCloseInLoop();

    void runSend(const base::Buf &buf);

private:
    DefaultCallback connectionCallback;
    MessageCallback messageCallback;

    std::shared_ptr<FdEventWatcher> watcher;
    std::atomic<StateE> state;
    size_t triggerByteNum;

    std::mutex writeLock;
    size_t waitSendByteNum;
    size_t waitSendCacheByteNum;
    size_t finishedSendByteNum;
};

bool SocketChannel::isConnected()
{
    return state == kConnected;
}

void SocketChannel::setConnectionCallback(const DefaultCallback &connectionCallback)
{
    this->connectionCallback = connectionCallback;
}

inline void SocketChannel::setNextTriggerByteNum(size_t triggerByteNum)
{
    this->triggerByteNum = triggerByteNum;
}

void SocketChannel::setMessageCallback(const MessageCallback &messageCallback)
{
    this->messageCallback = messageCallback;
}

inline void SocketChannel::setState(StateE state)
{
    this->state = state;
}

} // namespace hulatang::io
#endif // MYNET_TCP_CONNECTION_HPP_