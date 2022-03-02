#include "hulatang/io/CycleEventManager.hpp"

namespace hulatang::io {

void CycleEventManager::process() {}

bool CycleEventManager::isIdle() const noexcept
{
    return true;
}

} // namespace hulatang::io