#pragma once
#include "meta.hpp"
#include <date/date.h>
#include <variant>
#include <string>
#include <chrono>
#include <string_view>
#include <cstdlib>
#include <sstream>
#include <iostream>


namespace stanon {

enum class data_type {
    integer,
    floating_point,
    date,
    string,
    enumeration,
    null
};

using date_type = date::sys_time<std::chrono::seconds>;


struct null_type {};

std::ostream& operator<<(std::ostream& os, null_type) {
    return os << "(null)";
}

constexpr bool operator == (null_type, null_type) noexcept { return true; }
constexpr bool operator<(null_type, null_type) noexcept { return false; }


using typed_value = std::variant<long, double, date_type, std::string, null_type>;

std::ostream& operator<<(std::ostream& os, const typed_value& tv) {
    // doesn't work with clang 5+ and libstdc++: https://bugs.llvm.org/show_bug.cgi?id=33222
    std::visit([&os](const auto& value) {
        os << value;
    }, tv);
    return os;
}

using data_typeid_to_type = meta::map<
        meta::pair<meta::value<data_type::integer>, long>,
        meta::pair<meta::value<data_type::floating_point>, double>,
        meta::pair<meta::value<data_type::date>, date_type>,
        meta::pair<meta::value<data_type::string>, std::string>,
        meta::pair<meta::value<data_type::enumeration>, std::string>,
        meta::pair<meta::value<data_type::null>, null_type>
        >;

template<data_type TypeID>
using underlying_type = data_typeid_to_type::at<TypeID>;

static_assert(std::is_same_v<underlying_type<data_type::integer>, long>);
static_assert(std::is_same_v<underlying_type<data_type::floating_point>, double>);
static_assert(std::is_same_v<underlying_type<data_type::date>, date_type>);
static_assert(std::is_same_v<underlying_type<data_type::string>, std::string>);
static_assert(std::is_same_v<underlying_type<data_type::enumeration>, std::string>);
static_assert(std::is_same_v<underlying_type<data_type::null>, null_type>);


template<data_type type>
underlying_type<type> parse(std::string_view sv);

template<>
long parse<data_type::integer>(std::string_view sv) {
    return std::strtol(sv.data(), nullptr, /* base: */ 10);
}

template<>
double parse<data_type::floating_point>(std::string_view sv) {
    return std::strtof(sv.data(), nullptr);
}

template<>
date_type parse<data_type::date>(std::string_view sv) {
    using namespace std::literals;
    std::istringstream iss{ std::string{ sv } };
    date_type result;
    iss >> date::parse("%Y-%m-%d"s, result);
    return result;
}

template<>
std::string parse<data_type::string>(std::string_view sv) {
    return std::string{ sv};
}

template<>
std::string parse<data_type::enumeration>(std::string_view sv) {
    return std::string{ sv};
}

template<>
constexpr null_type parse<data_type::null>(std::string_view) {
    return {};
}

typed_value parse_tv(data_type type, std::string_view sv) {
    switch (type) {
        case data_type::integer:            return typed_value{ parse<data_type::integer>(sv)};
        case data_type::floating_point:     return typed_value{ parse<data_type::floating_point>(sv)};
        case data_type::date:               return typed_value{ parse<data_type::date>(sv)};
        case data_type::string:             return typed_value{ parse<data_type::string>(sv)};
        case data_type::enumeration:        return typed_value{ parse<data_type::enumeration>(sv)};
        case data_type::null:               return typed_value{ parse<data_type::null>(sv) };
    }
    // unreachable
    return {};
}


} // namespace stanon
