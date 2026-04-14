#ifndef PARSING_PARSING_HPP
#define PARSING_PARSING_HPP

#include <cJSON.h>

#include <chrono>
#include <expected>
#include <string_view>
#include <tuple>

namespace bin_unicorn {

/// @brief A type of bin collection.
enum class CollectionType : uint8_t {
    Unspecified = 0,
    DomesticWaste,
    FoodWaste,
    Recycling,
    GardenWaste
};

/// @brief A data object representing a bin collection.
struct BinCollection {
    std::chrono::year_month_day date;
    CollectionType collection_type;
};

using BinCollectionPair = std::tuple<BinCollection, BinCollection>;

/// @brief Represents the result of trying to parse a bin collection.
enum class ParseError {
    /// @brief The provided JSON was malformed and could not be parsed by cJSON.
    InvalidJson = -1,
    /// @brief The provided JSON did not match the expected object format.
    InvalidJsonSchema = -2
};

/// @brief Attempt to parse the two first bin collections in the array.
/// @param response_body A string_view of the JSON response body returned from the RBC API.
/// @returns A value indicating parse status (failure/success).
std::expected<BinCollectionPair, ParseError> parse_json_response(std::string_view response_body);

} // namespace bin_unicorn

#endif // PARSING_PARSING_HPP