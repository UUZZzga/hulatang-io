#include "hulatang/io/FdEventManager.hpp"

#include <thread>

namespace hulatang::io {
void FdEventManager::process(microseconds blockTime)
{
    std::this_thread::sleep_for(blockTime);
}
} // namespace hulatang::io