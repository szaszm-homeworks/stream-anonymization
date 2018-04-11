#pragma once

#include "common_types.hpp"
#include "col_type.hpp"
#include <vector>

namespace stanon {

struct table_data {
    std::vector<col_type>& schema;
    std::vector<std::vector<typed_value>> data; // first dimension: row, second dimension: attribute

    void apply_rules() {
        for (auto& row : data) {
            for (size_t i = 0; i < schema.size(); i++) {
                const col_type& ct = schema[i];
                row[i] = ct.apply_rules(row[i]);
            }
        }
    }
};

void print_row(std::ostream& os, const std::vector<typed_value>& row) {
    bool first = true;
    for (const auto& elem : row) {
        using stanon::operator<<;
        if (!first) os << ',';
        os << elem;
        first = false;
    }
    os << '\n';
}

} // namespace stanon
