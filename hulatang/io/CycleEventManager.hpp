#ifndef HULATANG_IO_CYCLEEVENTMANAGER_HPP
#define HULATANG_IO_CYCLEEVENTMANAGER_HPP

namespace hulatang::io {
class CycleEventManager
{
public:
    void process();

    [[nodiscard]] bool isIdle() const noexcept;
};
} // namespace hulatang::io

#endif // HULATANG_IO_CYCLEEVENTMANAGER_HPP
