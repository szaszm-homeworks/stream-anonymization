#pragma once
#include <tuple>
#include <type_traits>

namespace stanon::meta {

template<typename T>
struct type_type {
    using type = T;
};

template<typename T>
struct what_type;

template<typename First, typename Second>
struct pair {
    using first = First;
    using second = Second;
};

template<typename Pair>
using pair_first = typename Pair::first;

template<typename Pair>
using pair_second = typename Pair::second;

template<auto Val>
struct value {
    using type = decltype(Val);
    inline static constexpr type val = Val;
};

namespace detail {
template<typename First, typename...>
struct front_impl : type_type<First> {};
} // namespace detail

template<typename... Args>
using front = typename detail::front_impl<Args...>::type;

template<typename, typename... Types>
struct pop_front {
    template<template<typename...> typename Func>
    using call = Func<Types...>;
};

template<typename T, template<typename...> typename Func>
struct bind1st {
    template<typename... Args>
    using result = Func<T, Args...>;
};

template<typename...>
using false_t = std::false_type;

template<typename... Pairs>
class map {
public:
    struct not_found {};

private:
    template<typename... Args>
    using front_first_t = typename front<Args...>::first;

    using key_type = typename front_first_t<Pairs...>::type;

    static_assert((std::is_same_v<key_type, typename pair_first<Pairs>::type> && ...), "map key types are consistent");

    using storage = std::tuple<Pairs...>;

    template<typename...>
    struct find_impl;

    template<typename Key, typename... Prs>
    struct find_impl<Key, Prs...> {
        using type = std::conditional_t<std::is_same_v<typename front<Prs...>::first, Key>, typename front<Prs...>::second,
                typename pop_front<Prs...>::template call<bind1st<Key, find_impl>::template result>::type>;
    };

    template<typename Key>
    struct find_impl<Key> : type_type<not_found> {};

    template<typename... Args>
    using find = typename find_impl<Args...>::type;

public:
    template<key_type Key>
    using at = find<value<Key>, Pairs...>;
};

} // namespace stanon::meta
