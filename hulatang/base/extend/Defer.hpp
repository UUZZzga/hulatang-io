#ifndef HULATANG_BASE_EXTEND_DEFER_HPP
#define HULATANG_BASE_EXTEND_DEFER_HPP

#include <functional>

#define _DEFINE_CAT(X, Y) X##Y

#define ___DEFER(X, Y) _DEFINE_CAT(X, Y)
#define __DEFER_NAME() ___DEFER(f__, __COUNTER__)

namespace hulatang::base::extend {
struct DEFER_BUILD
{
    template<class T>
    struct DEFER
    {
        using FUNC_TYPE = T;

        inline explicit DEFER(FUNC_TYPE func)
            : func_(func)
        {}
        inline explicit DEFER(DEFER &&other)
            : func_(other.func_)
        {}
        inline ~DEFER()
        {
            func_();
        }
        FUNC_TYPE func_;
    };

    inline auto operator<<(auto func)
    {
        return DEFER<decltype(func)>{func};
    }
};

template<>
struct DEFER_BUILD::DEFER<std::function<void()>>
{
    inline explicit DEFER(std::function<void()> &&func)
        : func_(std::move(func))
    {}
    inline explicit DEFER(DEFER<std::function<void()>> &&defer)
        : func_(std::move(defer.func_))
    {}
    inline ~DEFER()
    {
        func_();
    }
    std::function<void()> func_;
};
}

#define __DEFER_CLASS(X, Y)                                                                                                                \
    ::hulatang::base::extend::DEFER_BUILD X;                                                                                                 \
    auto Y = X <<

#define defer __DEFER_CLASS(__DEFER_NAME(), __DEFER_NAME())

#endif // HULATANG_BASE_EXTEND_DEFER_HPP
