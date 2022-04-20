#include "hulatang/io/FdEventManager.hpp"

#include <thread>

namespace hulatang::io {
FdEventManager::FdEventManager(EventLoop *loop)
    : loop(loop)
    , watchers(1024)
{}

void FdEventManager::process(microseconds blockTime)
{
    std::this_thread::sleep_for(blockTime);
}

void FdEventManager::add(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd)
{
    auto i = fd.getFd();
    if(watchers.size() <= i){
        watchers.resize(i * 2);
    }
    watchers[i] = watcher;
}

void FdEventManager::change(const base::FileDescriptor &fd) {
}

void FdEventManager::cancel(const FdEventWatcherPtr &watcher, const base::FileDescriptor &fd) {
    auto i = fd.getFd();
    watchers[i].reset();
}

void FdEventManager::wakeup() {}

FdEventWatcherPtr FdEventManager::getWatcher(const base::FileDescriptor &fd) const {
    auto i = fd.getFd();
    return watchers[i];
}
} // namespace hulatang::io