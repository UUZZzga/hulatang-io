#include "hulatang/io/FdEventManager.hpp"

#include <cassert>
#include <thread>

namespace hulatang::io {
FdEventManager::FdEventManager(EventLoop *loop)
    : loop(loop)
    , channels(1024)
{}

void FdEventManager::process(microseconds blockTime)
{
    std::this_thread::sleep_for(blockTime);
}

void FdEventManager::add(Channel *channel)
{
    auto i = channel->getFd().getFd();
    if (channels.size() <= i)
    {
        channels.resize(i * 2);
    }
    assert(channels[i] == nullptr);
    channels[i] = const_cast<Channel *>(channel);
}

void FdEventManager::update(Channel *channel) {}

void FdEventManager::cancel(Channel *channel)
{
    auto i = channel->getFd().getFd();
    assert(channels[i] != nullptr);
    channels[i] = nullptr;
}

void FdEventManager::wakeup() {}
} // namespace hulatang::io