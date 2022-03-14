#ifndef HULATANG_IO_EXTEND_AWAITABLE_HPP
#define HULATANG_IO_EXTEND_AWAITABLE_HPP

#include "hulatang/io/extend/Promise.hpp"

namespace hulatang::io {
template<typename _Ty>
struct [[nodiscard]] awaitable_type
{
    typedef promise<awaitable_type<_Ty>, _Ty> promise_type;
    typedef promise_type::return_type return_type;

    std::coroutine_handle<promise_type> coro;

    awaitable_type(std::coroutine_handle<promise_type> &&coro)
        : coro(std::move(coro))
    {}

    template<class _PromiseT, typename = std::enable_if_t<is_promise_v<_PromiseT>>>
    void await_suspend(std::coroutine_handle<_PromiseT> handle)
    {
        auto loop = Scheduler::getSchedulerOfCurrentThread();
        assert(loop && "请运行 Scheduler");
        assert(static_cast<bool>(coro));
        assert(static_cast<bool>(handle));

        coro.promise().next = handle.address();
    }

    bool await_ready() const
    {
        return false;
    }

    return_type await_resume()
    {
#ifdef ENB_TASK_SWITCH_NUM
        task_switch_num.fetch_add(1);
#endif
        if constexpr (!std::is_void<return_type>())
        {
            return std::get<return_type>(coro.promise().result);
        }
    }
};

template<typename _Ty = void>
using awaitable = awaitable_type<_Ty>;

template<class _Ty>
struct is_awaitable : std::false_type
{};
template<class _Ty>
struct is_awaitable<awaitable<_Ty>> : std::true_type
{};
template<class _Ty>
constexpr bool is_awaitable_v = is_awaitable<std::remove_cvref_t<_Ty>>::value;

template<class _Ty>
concept awaitable_concept = is_awaitable_v<_Ty>;
}

#endif // HULATANG_IO_EXTEND_AWAITABLE_HPP
