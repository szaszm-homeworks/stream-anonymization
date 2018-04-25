#pragma once

#include "common_types.hpp"
#include "distribution.hpp"
#include <range/v3/core.hpp>
#include <range/v3/range_concepts.hpp>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <iostream>

namespace stanon {

enum class identification_class {
    identifier,
    quasi_identifier,
    non_identifier
};

struct col_type {
    std::string name;
    data_type type;
    distribution dist;
    std::vector<std::function<typed_value(typed_value input_value)>> rules;
    identification_class id_class;
    bool is_sensitive;

    typed_value apply_rules(typed_value in_value) const {
        typed_value working_value = std::move(in_value);
        for (auto mapper_func : rules) {
            auto result = mapper_func(std::move(working_value));
            working_value = std::move(result);
        }
        return working_value;
    }
};

template<typename Rng,
        CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>() && ranges::ConvertibleTo<ranges::range_value_type_t<Rng>, col_type>())>
void print_header(std::ostream& os, const Rng& schema) {
    bool first = true;
    for (const auto& col : schema) {
        using stanon::operator<<;
        if (!first) os << ',';
        os << col.name;
        first = false;
    }
    os << '\n';
}

} // namespace stanon
