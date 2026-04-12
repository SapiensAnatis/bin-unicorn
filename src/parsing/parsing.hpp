#ifndef PARSING_PARSING_H_
#define PARSING_PARSING_H_

#include <cJSON.h>

#include <expected>
#include <tuple>
#include <string_view>
#include <cstdint>

namespace parsing {

struct Date {
    uint16_t year;
    uint8_t month;
    uint8_t day;
};

/// @brief A type of bin collection.
enum class CollectionType : uint8_t { DomesticWaste, FoodWaste, Recycling, GardenWaste };

/// @brief A data object representing a bin collection.
struct BinCollection {
    Date date;
    CollectionType collection_type;
};

typedef std::tuple<BinCollection, BinCollection> BinCollectionPair;

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
std::expected<BinCollectionPair, ParseError> parse_response(const std::string_view &response_body);

} // namespace parsing

#endif // PARSING_PARSING_H_