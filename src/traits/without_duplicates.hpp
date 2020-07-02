#pragma once

#include "traits/contains.hpp"

#include <tuple>
#include <type_traits>


template <template <typename...> typename C, class Out, class In>
struct filter;

template <template <typename...> typename C, class... Out, class InCar, class... InCdr>
struct filter<C, C<Out...>, C<InCar, InCdr...>>
{
  using type = typename std::conditional<
    contains<C, C<Out...>, InCar>::value
    , typename filter<C, C<Out...>, C<InCdr...>>::type
    , typename filter<C, C<Out..., InCar>, C<InCdr...>>::type
  >::type;
};

template <template <typename...> typename C, class Out>
struct filter<C, Out, C<>>
{
  using type = Out;
};


template <template <typename...> typename C, class T>
using without_duplicates = typename filter<C, C<>, T>::type;
