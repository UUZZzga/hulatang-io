#ifndef A5A0DBF1_6232_4284_BF31_25B4737F6B5C
#define A5A0DBF1_6232_4284_BF31_25B4737F6B5C
#include "hulatang/base/Buf.hpp"
#include "hulatang/base/Buffer.hpp"
#include "hulatang/base/Log.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/Model.hpp"

#include <atomic>
#include <cassert>
#include <deque>
#include <memory>
#include <mutex>

namespace hulatang::io {
class Reactor : public Model
{
public:
    void handleEvent(Channel *channel) override;

    virtual void handleRead(Channel *channel) = 0;
    virtual void handleWrite(Channel *channel) = 0;
};
} // namespace hulatang::io

#endif // HULATANG_IO_SOCKETCHANNEL_HPP
