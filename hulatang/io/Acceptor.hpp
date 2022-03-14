#ifndef HULATANG_IO_ACCEPTOR_HPP_
#define HULATANG_IO_ACCEPTOR_HPP_

#include "hulatang/base/File.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include "hulatang/io/InetAddress.hpp"

#include <functional>
#include <string_view>

namespace hulatang::io {
class EventLoop;
class EventLoopThreadPool;
class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(base::FileDescriptor, FdEventWatcherPtr, InetAddress)>;
    Acceptor(EventLoop *_loop, std::string_view listenAddr, int port);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback = cb;
    }

    void listen();

    bool listening() const
    {
        // return listening_;
    }

    void setThreadPool(EventLoopThreadPool *threadPool_) { threadPool = threadPool_; }

private:
    void acceptInLoop();

private:
    EventLoop *loop;
    EventLoopThreadPool *threadPool;
    base::FileDescriptor acceptFd;
    base::FileDescriptor newFd;
    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io
#endif // HULATANG_IO_ACCEPTOR_HPP_
