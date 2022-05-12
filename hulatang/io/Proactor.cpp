#include "hulatang/io/Proactor.hpp"
#include "hulatang/io/Channel.hpp"
#include "hulatang/io/EventLoop.hpp"

namespace hulatang::io {

void Proactor::handleEvent(Channel *channel)
{
    auto tie = channel->getTie();
    auto *over = static_cast<Over *>(channel->rover);
    channel->rover = nullptr;

    auto *overNext = over->next;

    auto func = [this, channel](auto *over) {
        // TODO
        switch (over->type)
        {
        case Type::NONE: {
            delete over;
        }
        break;
        // case Type::OPEN:
        // case Type::CLOSE:
        //     abort();
        //     break;
        case Type::READ: {
            handleRead(channel, over);
        }
        break;
        case Type::WRITE: {
            handleWrite(channel, over);
        }
        break;
        case Type::UPDATE: {
            // channel update
            updatePost(channel);
        }
        break;
        default:
            abort();
        }
    };

    if (over != nullptr && overNext == nullptr)
    {
        func(over);
        over = nullptr;
    }
    while (overNext != nullptr)
    {
        over->next = nullptr;
        func(over);
        over = overNext;
        overNext = overNext->next;
    }
    if (over != nullptr)
    {
        func(over);
    }
}

void Proactor::addOver(Channel *channel, Over *over)
{
    Over *overPtr = static_cast<Over *>(channel->over);
    if (overPtr == nullptr)
    {
        channel->over = over;
        return;
    }
    while (overPtr->next != nullptr)
    {
        overPtr = overPtr->next;
    }
    overPtr->next = over;
}

void Proactor::addFree(Over *over)
{
    EventLoop::getEventLoopOfCurrentThread()->queueInLoop([over]() { delete over; });
}

static void FreeOverList(Proactor::Over *over)
{
    if (over == nullptr)
    {
        return;
    }
    if (over->next == nullptr)
    {
        delete over;
        return;
    }
    FreeOverList(over->next);
}

void FreeOverList(Channel *channel)
{
    auto *over = static_cast<Proactor::Over *>(channel->getOver());
    auto *rover = static_cast<Proactor::Over *>(channel->getROver());
    FreeOverList(over);
    FreeOverList(rover);
}

} // namespace hulatang::io