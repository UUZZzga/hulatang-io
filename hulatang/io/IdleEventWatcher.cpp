#include "hulatang/io/IdleEventWatcher.hpp"

namespace hulatang::io {

void IdleEventWatcher::handle()
{
    handler();
}

} // namespace hulatang::io