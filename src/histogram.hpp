#pragma once
#include "common_types.hpp"
#include <algorithm>
#include <functional>
#include <map>

namespace stanon {

template<typename T>
decltype(auto) identity_func(T&& arg) { return std::forward<T>(arg); }


class histogram {
    size_t n_values = 0;

    // this function should reduce the range of the values to a narrower, finite set
    std::function<typed_value(typed_value)> projection;
    std::map<typed_value, size_t> value_occurences;
public:
    explicit histogram(std::function<typed_value(typed_value)> projection_func = [](auto id){ return id;  })
        :projection{ std::move(projection_func) }
    {    }

    void add_value(typed_value value) {
        value_occurences[projection(std::move(value))]++;
        n_values++;
    }

    double probability(const typed_value& value) const {
        auto val_it = std::find_if(std::begin(value_occurences), std::end(value_occurences),
                [&value](const auto& pair) { return pair.second; });
        if(val_it == std::end(value_occurences)) { return 0; }
        if(n_values == 0) { return 0; }
        return static_cast<double>(value_occurences.at(value)) / n_values;
    }
};

} // namespace stanon

