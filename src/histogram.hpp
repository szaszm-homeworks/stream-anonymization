#pragma once
#include "common_types.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <cmath>

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
    { }

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

    const auto& get_value_occurences() const { return value_occurences; }
};

struct distance_exception : std::runtime_error {
    using std::runtime_error::runtime_error;
};
/*
auto earth_movers_distance(const histogram& h1_, const histogram& h2_) {
    const auto&[2] h = {
        h1_.get_value_occurences(),
        h2_.get_value_occurences()
    };

    typed_value max[2];
    for(size_t i = 0; i < 2; ++i) {
        if(h[i].size() < 1) {
            max[i] = 0;
        } else {
            auto largest_it = h[i].cend();
            --largest_it;
            max[i] = *largest_it;
        }
    }
    auto upper = std::max(max);

    typed_value min[2];
    for(size_t i = 0; i < 2; ++i) {
        if(h[i].size() < 1) {
            min[i] = 0;
        } else {
            auto largest_it = h[i].cend();
            --largest_it;
            max[i] = *largest_it;
        }
    }

    auto size_max = std::max({ h[0].size(), h[1].size() });

    auto it1 = h[0].cbegin();
    auto it2 = h[1].cbegin();
    typed_value current_sum = 0;
    typed_value result = 0;
    while(it1 != h[0].cend() || it2 != h[1].cend()) {
        if(it1 != h[0].cend() && it2 != h[1].cend()) {
            current_sum += *it1 - *it2;

        }

        if(it1 != h[0].cend()) ++it1;
        if(it2 != h[1].cend()) ++it2;
    }
} */

} // namespace stanon

