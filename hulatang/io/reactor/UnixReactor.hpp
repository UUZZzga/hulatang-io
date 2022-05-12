#ifndef HULATANG_IO_REACTOR_UNIXREACTOR_HPP
#define HULATANG_IO_REACTOR_UNIXREACTOR_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/io/Reactor.hpp"
namespace hulatang::io {
class UnixReactor : public Reactor
{
public:
    using MessageCallback = std::function<void()>;
    using WriteCallback = std::function<void(const char *, size_t)>;

    UnixReactor(base::Buffer *readBuf, base::Buffer *writeBuf);

    void handleRead(Channel *channel) override;
    void handleWrite(Channel *channel) override;

    void setMessageCallback(const MessageCallback &messageCallback_) { messageCallback = messageCallback_; }

    void setWriteCallback(const WriteCallback &writeCallback_) { writeCallback = writeCallback_; }

private:
    base::Buffer *readBuf;
    base::Buffer *writeBuf;
    MessageCallback messageCallback;
    WriteCallback writeCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_REACTOR_UNIXREACTOR_HPP
