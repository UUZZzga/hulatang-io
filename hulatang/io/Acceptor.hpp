#ifndef HULATANG_IO_ACCEPTOR_HPP
#define HULATANG_IO_ACCEPTOR_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/io/Model.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include <functional>
#include <memory>

namespace hulatang::io {
class EventLoop;
class EventLoopThreadPool;
class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(std::unique_ptr<Channel>, InetAddress)>;
    Acceptor(EventLoop *_loop, const InetAddress &address);
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

    void setThreadPool(EventLoopThreadPool *threadPool_)
    {
        threadPool = threadPool_;
    }

private:
    void acceptInLoop(base::socket::fd_t fd, InetAddress addr);

private:
    EventLoop *loop;
    EventLoopThreadPool *threadPool;
    base::FileDescriptor acceptFd;
    base::FileDescriptor idleFd;
    std::unique_ptr<Channel> acceptChannel;
    std::unique_ptr<Model> model;
    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_ACCEPTOR_HPP
