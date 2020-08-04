#pragma once

#include <type_traits>

#if defined(_MSC_VER)
// It is removed on MSVC, deprecated on gcc/clang
namespace std
{
    template <typename>
    struct result_of;

    template <typename F, typename... Args>
    struct result_of<F(Args...)> : std::invoke_result<F, Args...> {};
}
#endif
