#include "parsing/parsing.hpp"

#include "parsing/arena.hpp"
#include "util.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <expected>
#include <memory>
#include <string_view>

namespace bin_unicorn {

struct cJSONDeleter {
    void operator()(cJSON *ptr) { cJSON_Delete(ptr); }
};

static bool try_parse_collection_string(const std::string_view &service_string,
                                        CollectionType &out_collection_type) {
    size_t first_word_end = service_string.find(" Collection Service");
    if (first_word_end == std::string_view::npos) {
        return false;
    }

    std::string_view collection_type = service_string.substr(0, first_word_end);
    if (collection_type == "Domestic Waste") {
        out_collection_type = CollectionType::DomesticWaste;
        return true;
    } else if (collection_type == "Food Waste") {
        out_collection_type = CollectionType::FoodWaste;
        return true;
    } else if (collection_type == "Recycling") {
        out_collection_type = CollectionType::Recycling;
        return true;
    } else if (collection_type == "Garden Waste") {
        out_collection_type = CollectionType::GardenWaste;
        return true;
    }

    return false;
}

static bool try_parse_date(const std::string_view &date_time_string, Date &out_date) {
    // We receive dates in the format DD/MM/YYYY 00:00:00
    // We don't care about the time, but need at least "DD/MM/YYYY" (10 chars)

    if (date_time_string.size() < 10) {
        return false;
    }

    uint8_t day = 0;
    if (!try_parse_number(date_time_string.substr(0, 2), day)) {
        return false;
    }

    uint8_t month = 0;
    if (!try_parse_number(date_time_string.substr(3, 2), month)) {
        return false;
    }

    uint16_t year = 0;
    if (!try_parse_number(date_time_string.substr(6, 4), year)) {
        return false;
    }

    out_date = Date{.year = year, .month = month, .day = day};

    return true;
}

static std::expected<BinCollection, ParseError> parse_collection(const cJSON *collection) {
    cJSON *date = cJSON_GetObjectItem(collection, "Date");
    if (!cJSON_IsString(date)) {
        fprintf(stderr, "Error parsing JSON: $.Collections[0].Date was not a string\n");
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    cJSON *service = cJSON_GetObjectItem(collection, "Service");
    if (!cJSON_IsString(service)) {
        fprintf(stderr, "Error parsing JSON: $.Collections[0].Service was not a string\n");
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    Date parsed_date;
    if (!try_parse_date(std::string_view(date->valuestring), parsed_date)) {
        fprintf(stderr, "Error parsing JSON: $.Collections[0].Date '%s' was not a valid date\n",
                date->valuestring);
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    CollectionType parsed_collection_type;
    if (!try_parse_collection_string(std::string_view(service->valuestring),
                                     parsed_collection_type)) {
        fprintf(stderr,
                "Error parsing JSON: $.Collections[0].Service '%s' did not match expected format\n",
                service->valuestring);
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    BinCollection out_bin_collection = {};

    out_bin_collection.date = parsed_date;
    out_bin_collection.collection_type = parsed_collection_type;

    return out_bin_collection;
}

bool operator==(const Date &a, const Date &b) {
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

static std::expected<BinCollectionPair, ParseError>
parse_json_response_inner(const std::string_view response_body) {
    auto json = std::unique_ptr<cJSON, cJSONDeleter>{
        cJSON_ParseWithLength(response_body.data(), response_body.size())};

    if (json.get() == nullptr) {
        const char *error_ptr = cJSON_GetErrorPtr();
        fprintf(stderr, "Error parsing JSON: parse failure at %s\n", error_ptr);
        return std::unexpected(ParseError::InvalidJson);
    }

    cJSON *collections = cJSON_GetObjectItem(json.get(), "Collections");
    if (!cJSON_IsArray(collections)) {
        fprintf(stderr, "Error parsing JSON: $.Collections was not an array\n");
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    cJSON *first_collection = cJSON_GetArrayItem(collections, 0);
    if (!cJSON_IsObject(first_collection)) {
        fprintf(stderr, "Error parsing JSON: $.Collections[0] was not an object\n");
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    auto first_parse_result = parse_collection(first_collection);
    if (!first_parse_result.has_value()) {
        fprintf(stderr, "Failed to parse first collection\n");
        return std::unexpected(first_parse_result.error());
    }

    cJSON *second_collection = cJSON_GetArrayItem(collections, 1);
    if (!cJSON_IsObject(second_collection)) {
        fprintf(stderr, "Error parsing JSON: $.Collections[1] was not an object\n");
        return std::unexpected(ParseError::InvalidJsonSchema);
    }

    auto second_parse_result = parse_collection(second_collection);
    if (!second_parse_result.has_value()) {
        fprintf(stderr, "Failed to parse second collection\n");
        return std::unexpected(second_parse_result.error());
    }

    return BinCollectionPair{*first_parse_result, *second_parse_result};
}

std::expected<BinCollectionPair, ParseError>
parse_json_response(const std::string_view response_body) {
    alignas(max_align_t) std::array<std::byte, 4096> arena_buffer{};

    auto arena = Arena{arena_buffer.data(), arena_buffer.size()};
    arena_set_current(arena);

    // It's not thread-safe to call cJSON_InitHooks more than once, but we are only dealing with
    // cJSON from one core.
    cJSON_Hooks hooks{.malloc_fn = arena_malloc, .free_fn = arena_free};
    cJSON_InitHooks(&hooks);

    auto result = parse_json_response_inner(response_body);

    cJSON_InitHooks(nullptr); // Reset hooks
    arena_unset_current();

    return result;
}

} // namespace bin_unicorn