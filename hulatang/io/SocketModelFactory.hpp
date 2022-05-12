#ifndef HULATANG_IO_SOCKETMODELFACTORY_HPP
#define HULATANG_IO_SOCKETMODELFACTORY_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/io/Model.hpp"

#include <functional>
#include <memory>

struct sockaddr;

namespace hulatang::io {
// accept/connect 成功后创建只能recv/send的Model
class SocketModelFactory
{
public:
    using MessageCallback = std::function<void()>;
    using WriteCallback = std::function<void(const char*, size_t)>;

    static std::unique_ptr<Model> create(Channel *channel, base::Buffer *recvBuf, base::Buffer *sendBuf, MessageCallback messageCallback, WriteCallback writeCallback);
    static std::unique_ptr<Model> createConnect(Channel *channel, sockaddr *addr, size_t addrLen);
};
} // namespace hulatang::io

#endif // HULATANG_IO_SOCKETMODELFACTORY_HPP
