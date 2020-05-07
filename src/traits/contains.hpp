#pragma once

#include <tuple>
#include <type_traits>


template <class Haystack, class Needle>
struct contains;

template <class Car, class... Cdr, class Needle>
struct contains<std::tuple<Car, Cdr...>, Needle> : contains<std::tuple<Cdr...>, Needle>
{};

template <class... Cdr, class Needle>
struct contains<std::tuple<Needle, Cdr...>, Needle> : std::true_type
{};

template <class Needle>
struct contains<std::tuple<>, Needle> : std::false_type
{};
