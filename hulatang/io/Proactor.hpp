#ifndef HULATANG_IO_ProactorChannel_HPP
#define HULATANG_IO_ProactorChannel_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/io/Model.hpp"

namespace hulatang::io {
class Channel;

enum class Type
{
    NONE,
    OPEN,
    CLOSE,
    READ,
    WRITE,
    UPDATE,
};

void FreeOverList(Channel* channel);

class Proactor : public Model
{
public:
    struct Over
    {
        Over *next{};
        Type type{Type::NONE};
    };

    // handleEvent 不负责释放 over
    void handleEvent(Channel *channel) override;

    virtual void handleRead(Channel *channel, Over *over) = 0;
    virtual void handleWrite(Channel *channel, Over *over) = 0;

protected:
    virtual void updatePost(Channel *channel) {}
    void addOver(Channel *channel, Over *over);
    void addFree(Over *over);
};
} // namespace hulatang::io

#endif // HULATANG_IO_ProactorChannel_HPP
