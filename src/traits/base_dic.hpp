#pragma once

#include <tuple>
#include <type_traits>


template <class Candidate, class In>
struct base_dic;

template <class Candidate, class InCar, class... InCdr>
struct base_dic<Candidate, std::tuple<InCar, InCdr...>>
{
  using type = typename std::conditional<
    std::is_same<Candidate, typename InCar::derived_t>::value
    , typename base_dic<InCar, std::tuple<>>::type
    , typename base_dic<Candidate, std::tuple<InCdr...>>::type
  >::type;
};

template <class Candidate>
struct base_dic<Candidate, std::tuple<>>
{
  using type = Candidate;
};
