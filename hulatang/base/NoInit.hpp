#ifndef HULATANG_BASE_NOINIT_HPP
#define HULATANG_BASE_NOINIT_HPP

#include <type_traits>
#include <utility>

namespace hulatang::base {
// https://stackoverflow.com/questions/96579/stl-vectors-with-uninitialized-storage
template<typename T>
struct NoInit
{
    T value;

    NoInit()
    {
        static_assert(std::is_standard_layout<NoInit<T>>::value && sizeof(T) == sizeof(NoInit<T>), "T does not have standard layout");
    }

    explicit NoInit(T &v)
        : value(v)
    {}

    ~NoInit() = default;

    const T &operator=(T &v)
    {
        value = v;
        return value;
    }

    NoInit(NoInit<T> &n)
        : value(n.value)
    {}

    NoInit(NoInit<T> &&n) noexcept
        : value(std::move(n.value))
    {}

    const T &operator=(NoInit<T> &n)
    {
        value = n.value;
        return this;
    }

    const T &operator=(NoInit<T> &&n) noexcept
    {
        value = std::move(n.value);
        return this;
    }

    T *operator&()
    {
        return &value;
    } // So you can use &(vec[0]) etc.

    const T *operator&() const
    {
        return &value;
    }
};
} // namespace hulatang::base

#endif // HULATANG_BASE_NOINIT_HPP
