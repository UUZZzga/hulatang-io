#ifndef HULATANG_IO_CONNECTOR_HPP
#define HULATANG_IO_CONNECTOR_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/InetAddress.hpp"

#include <chrono>
#include <memory>
#include <string_view>

namespace hulatang::io {
class Connector : public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(base::FileDescriptor &, FdEventWatcherPtr)>;

    Connector(EventLoop *_loop, InetAddress &_address);
    ~Connector();

    void start();   // can be called in any thread
    void restart(); // must be called in loop thread
    void stop();    // can be called in any thread

    void setNewConnectionCallback(const NewConnectionCallback &newConnectionCallback_)
    {
        newConnectionCallback = newConnectionCallback_;
    }

private:
    void connect();

    void connecting();
    void connected();
    void retry();

    void startInLoop();
    void handleError(std::error_condition &ec);

private:
    enum class State
    {
        Disconnected,
        Connecting,
        Connected,
    };
    EventLoop *loop;
    InetAddress &address;
    std::chrono::milliseconds retryDelay;
    base::FileDescriptor fd;
    std::unique_ptr<Channel> channel;
    State state;
    FdEventWatcherPtr watcher;
    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_CONNECTOR_HPP
