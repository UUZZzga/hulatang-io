#ifndef HULATANG_IO_IDLEEVENTWATCHER_HPP
#define HULATANG_IO_IDLEEVENTWATCHER_HPP

#include "hulatang/io/EventWatcher.hpp"
#include <functional>

namespace hulatang::io {
class IdleEventWatcher : public EventWatcher
{
public:
    void handle();
};
} // namespace hulatang::io

#endif // HULATANG_IO_IDLEEVENTWATCHER_HPP
