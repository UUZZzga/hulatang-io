#ifndef HULATANG_IO_CYCLEEVENTWATCHER_HPP
#define HULATANG_IO_CYCLEEVENTWATCHER_HPP

#include "hulatang/io/EventWatcher.hpp"

namespace hulatang::io {
class CycleEventWatcher : public EventWatcher
{
public:
    using EventWatcher::EventWatcher;

    void handle();
};
} // namespace hulatang::io

#endif // HULATANG_IO_CYCLEEVENTWATCHER_HPP
