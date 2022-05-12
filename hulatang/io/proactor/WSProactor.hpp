#ifndef HULATANG_IO_PROACTOR_WSPROACTOR_HPP
#define HULATANG_IO_PROACTOR_WSPROACTOR_HPP

#include "hulatang/io/Proactor.hpp"
#include <functional>

namespace hulatang::io {
// Windows Socket IOCP
class WSProactor : public Proactor
{
public:
    using MessageCallback = std::function<void()>;
    using WriteCallback = std::function<void(const char *, size_t)>;

    WSProactor(Channel *channel, base::Buffer *recvBuf, base::Buffer *sendBuf);

    void handleRead(Channel *channel, Over *over) override;
    void handleWrite(Channel *channel, Over *over) override;

    void setMessageCallback(const MessageCallback &messageCallback_) { messageCallback = messageCallback_; }

    void setWriteCallback(const WriteCallback &writeCallback_) { writeCallback = writeCallback_; }

protected:
    void updatePost(Channel *channel) override;

private:
    void postRecv(Channel *channel, Over *over);
    void postSend(Channel *channel, Over *over);

    base::Buffer *readBuf;
    base::Buffer *writeBuf;
    MessageCallback messageCallback;
    WriteCallback writeCallback;
};
} // namespace hulatang::io

#endif // HULATANG_IO_PROACTOR_WSPROACTOR_HPP
