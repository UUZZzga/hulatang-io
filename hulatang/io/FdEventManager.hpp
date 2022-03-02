#ifndef HULATANG_IO_FDEVENTMANAGER_HPP
#define HULATANG_IO_FDEVENTMANAGER_HPP

#include <chrono>

namespace hulatang::io {
class FdEventManager
{
public:
    typedef std::chrono::microseconds microseconds;
    void process(microseconds blockTime);
};
} // namespace hulatang::io

#endif // HULATANG_IO_FDEVENTMANAGER_HPP
