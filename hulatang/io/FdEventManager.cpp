#include "hulatang/io/FdEventManager.hpp"

#include <thread>

namespace hulatang::io {
void FdEventManager::process(microseconds blockTime)
{
    std::this_thread::sleep_for(blockTime);
}

void FdEventManager::add(const FdEventWatcherPtr &watcher)
{
    watchers.emplace_back(watcher);
}
} // namespace hulatang::io