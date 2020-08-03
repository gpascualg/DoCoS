#pragma once

#include "updater/updater.hpp"
#include "traits/base_dic.hpp"
#include "traits/has_type.hpp"
#include "traits/without_duplicates.hpp"



template <typename... vectors>
struct scheme;

template <typename... vectors>
struct scheme_store;


namespace detail
{
    template <typename component, typename... Args>
    struct scheme_arguments
    {
        component& comp;
        std::tuple<Args...> args;
    };
}


template <typename... vectors>
struct scheme_store
{
    template <typename T> using dic_t = typename base_dic<T, std::tuple<vectors...>>::type;

    constexpr scheme_store()
    {}

    template <typename T>
    constexpr inline T& get()
    {
        return std::get<T>(components);
    }

    std::tuple<vectors...> components;
};

template <typename... vectors>
class scheme
{
    template <typename... T> friend class scheme;

public:
    std::tuple<std::add_lvalue_reference_t<vectors>...> components;

    constexpr updater<std::add_pointer_t<vectors>...> make_updater()
    {
        return updater<std::add_pointer_t<vectors>...>(components);
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

    template <typename... T, typename... D>
    constexpr auto overlap(scheme_store<T...>& store, scheme<D...>& other)
    {
        using W = without_duplicates<scheme, scheme<D..., vectors...>>;
        return W{ store };
    }

    template <typename... T>
    static constexpr inline auto make(scheme_store<T...>& store)
    {
        using W = without_duplicates<scheme, scheme<scheme_store<T...>::dic_t<vectors>...>>;
        return W{ store };
    }

private:
    template <typename... T>
    constexpr scheme(scheme_store<T...>& store) :
        components(store.get<vectors>()...)
    {}
};


template <typename... T, typename A, typename B, typename... O>
constexpr inline auto overlap(scheme_store<T...>& store, A&& a, B&& b, O&&... other)
{
    if constexpr (sizeof...(other) == 0)
    {
        return a.overlap(store, b);
    }
    else
    {
        return overlap(store, a.overlap(b), std::forward<O>(other)...);
    }
}
