#include "hulatang/io/FdEventManager.hpp"

#include <thread>

namespace hulatang::io {
FdEventManager::FdEventManager(EventLoop *loop)
    : loop(loop)
{}

void FdEventManager::process(microseconds blockTime)
{
    std::this_thread::sleep_for(blockTime);
}

void FdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd)
{
    watchers.emplace_back(watcher);
}
} // namespace hulatang::io