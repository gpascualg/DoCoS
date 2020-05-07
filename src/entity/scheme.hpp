#pragma once

#include "updater/updater.hpp"
#include "traits/base_dic.hpp"
#include "traits/has_type.hpp"
#include "traits/without_duplicates.hpp"




template <typename... vectors>
struct scheme;

namespace detail
{
    template <template <typename...> typename Tpl, typename... E> 
    constexpr inline auto make_scheme_impl(scheme<E...> sch, Tpl<E...> unused)
    {
        return sch;
    }

    template <typename T>
    struct scheme_store
    {
        inline static T component;
    };

    template <typename component, typename... Args>
    struct scheme_arguments
    {
        component& comp;
        std::tuple<Args...> args;
    };
}


template <typename... vectors>
class scheme
{
    template <typename... T> friend class scheme;

public:
    std::tuple<std::add_lvalue_reference_t<vectors>...> components;

    constexpr updater<std::add_pointer_t<vectors>...> make_updater(uint8_t num_threads)
    {
        return updater<std::add_pointer_t<vectors>...>(num_threads, components);
    }

    template <typename T>
    constexpr inline T& get()
    {
        return std::get<T&>(components);
    }

    template <typename T>
    constexpr inline bool has() const
    {
        return has_type<T, std::tuple<vectors...>>::value;
    }

    template <typename T>
    constexpr inline void require() const
    {
        static_assert(has_type<T, std::tuple<vectors...>>::value, "Requirement not met");
    }

    template <typename T, typename... Args>
    constexpr auto args(Args&&... args) -> detail::scheme_arguments<std::add_lvalue_reference_t<typename base_dic<T, std::tuple<vectors...>>::type>, decltype(args)...>
    {
        using D = typename base_dic<T, std::tuple<vectors...>>::type;
        require<D>();

        return {
            .comp = std::get<std::add_lvalue_reference_t<D>>(components),
            .args = std::forward_as_tuple(std::forward<Args>(args)...)
        };
    }

    template <typename... T>
    constexpr auto overlap(scheme<T...>& other)
    {
        using W = without_duplicates<std::tuple<T..., vectors...>>;
        return ::detail::make_scheme_impl({}, W{});
    }

    static constexpr inline auto make()
    {
        using W = without_duplicates<std::tuple<vectors...>>;
        return ::detail::make_scheme_impl({}, W{});
    }

private:
    constexpr scheme() :
        components(detail::scheme_store<vectors>::component...)
    {}
};


template <typename A, typename B, typename... O>
constexpr inline auto overlap(A&& a, B&& b, O&&... other)
{
    if constexpr (sizeof...(other) == 0)
    {
        return a.overlap(b);
    }
    else
    {
        return overlap(a.overlap(b), std::forward<O>(other)...);
    }
}
