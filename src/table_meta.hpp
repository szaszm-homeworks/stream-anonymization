#pragma once
#include "col_type.hpp"
#include <vector>

namespace stanon {

struct table_meta {
    int target_k;
    double target_t;
    std::vector<col_type>& schema;
};

} // namespace stanon
