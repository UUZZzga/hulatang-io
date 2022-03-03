#ifndef HULATANG_IO_IDLEEVENTMANAGER_HPP
#define HULATANG_IO_IDLEEVENTMANAGER_HPP

#include "hulatang/io/IdleEventWatcher.hpp"
#include <vector>
namespace hulatang::io {
class IdleEventManager
{
public:
    void process();

    void createEvent(const std::function<void()> &f);
    void createEvent(std::function<void()> &&f);

private:
    std::vector<IdleEventWatcher> idleWatchers;
};
} // namespace hulatang::io

#endif // HULATANG_IO_IDLEEVENTMANAGER_HPP
