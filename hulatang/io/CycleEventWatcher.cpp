#include "hulatang/io/CycleEventWatcher.hpp"

namespace hulatang::io {
void CycleEventWatcher::handle()
{
    handler();
}

} // namespace hulatang::io