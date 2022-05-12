#ifndef HULATANG_IO_CONNECTOR_HPP
#define HULATANG_IO_CONNECTOR_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/io/Model.hpp"
#include "hulatang/io/SocketModelFactory.hpp"

#include <chrono>
#include <memory>

namespace hulatang::io {
class Connector : public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(std::unique_ptr<Channel>)>;

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
    void handleError(const std::error_condition &ec);

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
    std::unique_ptr<Model> model;
    State state;
    NewConnectionCallback newConnectionCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_CONNECTOR_HPP
