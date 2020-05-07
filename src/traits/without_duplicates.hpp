#pragma once

#include "traits/contains.hpp"

#include <tuple>
#include <type_traits>


template <class Out, class In>
struct filter;

template <class... Out, class InCar, class... InCdr>
struct filter<std::tuple<Out...>, std::tuple<InCar, InCdr...>>
{
  using type = typename std::conditional<
    contains<std::tuple<Out...>, InCar>::value
    , typename filter<std::tuple<Out...>, std::tuple<InCdr...>>::type
    , typename filter<std::tuple<Out..., InCar>, std::tuple<InCdr...>>::type
  >::type;
};

template <class Out>
struct filter<Out, std::tuple<>>
{
  using type = Out;
};


template <class T>
using without_duplicates = typename filter<std::tuple<>, T>::type;
