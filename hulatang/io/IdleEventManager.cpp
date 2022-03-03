#include "hulatang/io/IdleEventManager.hpp"

#include "hulatang/base/Log.hpp"

namespace hulatang::io {

void IdleEventManager::process()
{
    DLOG_TRACE;
    for (auto &watcher : idleWatchers)
    {
        watcher.handle();
    }
}

void IdleEventManager::createEvent(const std::function<void()> &f)
{
    IdleEventWatcher watcher;
    watcher.setHandler(f);
    idleWatchers.emplace_back(watcher);
}

void IdleEventManager::createEvent(std::function<void()> &&f)
{
    IdleEventWatcher watcher;
    watcher.setHandler(std::move(f));
    idleWatchers.emplace_back(watcher);
}

} // namespace hulatang::io