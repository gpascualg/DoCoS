#pragma once

#include <tuple>
#include <type_traits>


template <template <typename...> typename C, class Haystack, class Needle>
struct contains;

template <template <typename...> typename C, class Car, class... Cdr, class Needle>
struct contains<C, C<Car, Cdr...>, Needle> : contains<C, C<Cdr...>, Needle>
{};

template <template <typename...> typename C, class... Cdr, class Needle>
struct contains<C, C<Needle, Cdr...>, Needle> : std::true_type
{};

template <template <typename...> typename C, class Needle>
struct contains<C, C<>, Needle> : std::false_type
{};
