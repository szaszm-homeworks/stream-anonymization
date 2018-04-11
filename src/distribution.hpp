#pragma once
#include "common_types.hpp"
#include <functional>

namespace stanon {

class distribution {
    typed_value min_;
    typed_value max_;
    std::function<typed_value(typed_value value) > probability_func_;
public:
    distribution() {}
    explicit distribution(typed_value min, typed_value max, std::function<typed_value(typed_value value) > probability_func)
    : min_{min}, max_{ max}, probability_func_{ probability_func}
    {}

    typed_value min() const { return min_; }
    typed_value max() const { return max_; }
    typed_value probability(typed_value value) { if(probability_func_) return probability_func_(value); else return typed_value{}; }
};

} // namespace stanon

