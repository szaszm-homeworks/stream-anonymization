#include "table_meta.hpp"
#include "col_type.hpp"
#include "distribution.hpp"
#include "table_data.hpp"
#include "common_types.hpp"
#include <range/v3/core.hpp>
#include <range/v3/range_concepts.hpp>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>



template<typename Rng, template<typename...> typename OutputRng = std::vector,
        CONCEPT_REQUIRES_(ranges::v3::ForwardRange<Rng>() && ranges::v3::ConvertibleTo<ranges::v3::range_value_type_t<Rng>, stanon::col_type>())>
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


int main() {
    using namespace stanon;
    //using namespace std::literals;

    std::vector<col_type> schema = {
        /* name                   type                dist rules        identification_class           is_sensitive */
        { "FullName",          data_type::string,       {}, {},   identification_class::identifier,        true },
        { "NativeCountry",     data_type::enumeration,  {}, {},   identification_class::non_identifier,    false },
        { "Gender",            data_type::enumeration,  {}, {},   identification_class::non_identifier,    false },
        { "Age",               data_type::integer,      {}, {},   identification_class::quasi_identifier,  true },
        { "MaritalStatus",     data_type::enumeration,  {}, {},   identification_class::non_identifier,    false },
        { "EconomicStatus",    data_type::enumeration,  {}, {},   identification_class::non_identifier,    true },
        { "IndustrialGroup",   data_type::enumeration,  {}, {},   identification_class::quasi_identifier,  false },
        { "HighestEducationCompleted",data_type::enumeration,{},{},identification_class::non_identifier,   false },
        { "FieldOfStudy",      data_type::enumeration,  {}, {},   identification_class::non_identifier,    false },
        { "County",            data_type::enumeration,  {}, {},   identification_class::quasi_identifier,  false }
    };

    //table_meta tm{ /* k: */ 3, /* t: */ 1.0, schema };
    table_data td{ schema, {} };

    {
        std::ifstream ifs{ "irishcensus100m.csv" };
        std::string line;
        std::getline(ifs, line); // skip header
        while (ifs && ifs.good()) {
            std::getline(ifs, line);
            if(!ifs) break;
            td.data.emplace_back(parse_csv_row(line, schema));
        }
    }

    schema[0].rules.emplace_back([](const auto&) { return ""; });
    schema[3].rules.emplace_back([](const auto& in) {
        auto age = std::get<long>(in);
        age = age - (age % 5);
        return fmt::format("{0}-{1}", age, age + 4);
    });
    auto truncate_string = [](auto&& in) { return fmt::format("{0}...", std::get<std::string>(in).substr(0, 3)); };
    schema[6].rules.emplace_back(truncate_string);
    schema[9].rules.emplace_back(truncate_string);

    td.apply_rules();


    {
        std::ofstream ofs{ "output.csv" };
        print_header(ofs, schema);
        for (auto row : td.data) {
            print_row(ofs, row);
        }
    }


    return 0;
}
