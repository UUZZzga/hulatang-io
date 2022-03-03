#include "hulatang/io/EventWatcher.hpp"

namespace hulatang::io {
void ClosableEventWatcher::cancel()
{
    if (cancelCallback)
    {
        cancelCallback();
    }
}
} // namespace hulatang::io
