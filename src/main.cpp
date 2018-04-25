#include "table_meta.hpp"
#include "col_type.hpp"
#include "histogram.hpp"
#include "table_data.hpp"
#include "common_types.hpp"
#include <range/v3/core.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/algorithm/min.hpp>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <map>
#if __has_include(<filesystem>)
# include <filesystem>
  namespace fs = std::filesystem;
#else
# include <boost/filesystem.hpp>
  namespace fs = boost::filesystem;
#endif


template<typename Rng, template<typename...> typename OutputRng = std::vector,
        CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>() && ranges::ConvertibleTo<ranges::range_value_type_t<Rng>, stanon::col_type>())>
OutputRng<stanon::typed_value> parse_csv_row(std::string_view line, const Rng& schema, char delimiter = ',') {
    using namespace stanon;
    OutputRng<typed_value> row;
    row.reserve(std::size(schema));
    size_t start = 0;
    for (auto col : schema) {
        assert(start != std::string_view::npos && start < line.size() && "invalid start index");
        auto end = line.find(delimiter, start);
        std::string_view attribute_str = line.substr(start, /* length: */ end - start);
        row.emplace_back(parse_tv(col.type, attribute_str));
        start = (end == std::string_view::npos) ? std::string_view::npos : end + 1;
    }
    return row;
}

template<typename Rng,
        CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>() && ranges::ConvertibleTo<ranges::range_value_type_t<Rng>, stanon::col_type>()) >
stanon::table_data read_data_from_csv_file(const fs::path& path, const Rng& schema) {
    stanon::table_data td{ schema, {} };
    std::ifstream ifs{ path.string() };
    std::string line;
    std::getline(ifs, line); // skip header
    while (ifs && ifs.good()) {
        std::getline(ifs, line);
        if (!ifs) break;
        td.data.emplace_back(parse_csv_row(line, schema));
    }

    return td;
}

template<typename Rng,
        CONCEPT_REQUIRES_(ranges::ForwardRange<Rng>() && ranges::ConvertibleTo<ranges::range_value_type_t<Rng>, stanon::typed_value>())>
std::ostream& print_tuple(std::ostream& os, const Rng& rng) {
    os << '(';
    bool first = true;
    for(const stanon::typed_value& val: rng) {
        if(!first) { os << ", "; }
        os << val;
        first = false;
    }
    return os << ')';
}


int main() {
    using namespace stanon;

    auto group_age = [](const auto& in) {
        constexpr auto group_size = 5;
        auto age = std::get<long>(in);
        age = age - (age % group_size);
        return fmt::format("{0}-{1}", age, age + group_size - 1);
    };

    std::vector<col_type> schema = {
        /* name                   type                hist rules                  identification_class           is_sensitive */
        { "FullName",          data_type::string,       histogram{},         {},   identification_class::identifier,        true },
        { "NativeCountry",     data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    false },
        { "Gender",            data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    false },
        { "Age",               data_type::integer,      histogram{ group_age }, {},   identification_class::quasi_identifier,  true },
        { "MaritalStatus",     data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    false },
        { "EconomicStatus",    data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    true },
        { "IndustrialGroup",   data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    false },
        { "HighestEducationCompleted",data_type::enumeration,histogram{},    {}, identification_class::non_identifier,   false },
        { "FieldOfStudy",      data_type::enumeration,  histogram{},         {},   identification_class::non_identifier,    false },
        { "County",            data_type::enumeration,  histogram{},         {},   identification_class::quasi_identifier,  false }
    };

    //table_meta tm{ /* k: */ 3, /* t: */ 1.0, schema };
    table_data td = read_data_from_csv_file("irishcensus100m.csv", schema);
    std::cout << "read data, records: " << td.data.size() << '\n';

    schema[0].rules.emplace_back([](const auto&) { return null_type{}; });
    schema[3].rules.emplace_back(group_age);
    auto truncate_string = [](auto&& in) { return fmt::format("{0}...", std::get<std::string>(in).substr(0, 3)); };
    schema[6].rules.emplace_back(truncate_string);
    schema[9].rules.emplace_back(truncate_string);

    td.apply_rules();


    // measure k-anonymity by tracking identity groups
    namespace view = ranges::view;
    std::map<std::vector<typed_value>, size_t> records_per_identity_group;

    // collect identifier and sensitive column indices
    std::vector<size_t> identifier_column_ids;
    std::vector<size_t> sensitive_column_ids;
    for(size_t i = 0; i < schema.size(); ++i) {
        if(schema[i].id_class != identification_class::non_identifier) {
            identifier_column_ids.push_back(i);
        }

        if(schema[i].is_sensitive) {
            sensitive_column_ids.push_back(i);
        }
    }

    auto make_id_tuple = [&identifier_column_ids](const std::vector<typed_value>& row_data) {
        std::vector<typed_value> result;
        result.reserve(identifier_column_ids.size());
        for(size_t idx: identifier_column_ids) {
            result.push_back(row_data[idx]);
        }
        return result;
    };


    for(const auto& row: td.data) {
        records_per_identity_group[make_id_tuple(row)]++;


    }

    auto k = ranges::min(records_per_identity_group | view::values);
    std::cout << "k: " << k << '\n';
    std::cout << "number of identity groups: " << records_per_identity_group.size() << '\n';

    // identity groups of size k
    std::cout << "k-sized identity groups:\n";
    auto minimal_identity_group_ids = records_per_identity_group
        | view::filter([k](const auto& p) { return p.second == k; })
        | view::keys;
    for(const auto& grp: minimal_identity_group_ids) {
        print_tuple(std::cout, grp) << '\n';
    }




    {
        std::ofstream ofs{ "output.csv" };
        print_header(ofs, schema);
        for (const auto& row : td.data) {
            print_row(ofs, row);
        }
    }


    return 0;
}
