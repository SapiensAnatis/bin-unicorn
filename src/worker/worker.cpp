#include "worker/worker.hpp"

#include "http/http.hpp"
#include "parsing/parsing.hpp"

#include <array>
#include <cstdio>
#include <expected>
#include <span>

namespace worker {

static constexpr uint32_t ERROR_SLEEP_MS = 10'000;

static constexpr uint32_t THREE_HOURS_MS = 3 * 60 * 60 * 1'000;
static constexpr uint32_t SUCCESS_SLEEP_MS = THREE_HOURS_MS * 2;

static constexpr WorkLoopResult FAIL_RESULT = {.sleep_time_ms = ERROR_SLEEP_MS,
                                               .next_collection = std::nullopt};

static constexpr size_t RESPONSE_BUFFER_SIZE = 2048;

WorkLoopResult do_work_loop() {
    static std::array<char, RESPONSE_BUFFER_SIZE>
        http_buffer_array{}; // should be zeroed, in case a fake Content-Length is sent
    std::span<char> http_buffer(http_buffer_array);

    auto fetch_result = http::fetch_collection_data(http_buffer);
    if (!fetch_result.has_value()) {
        printf("Failed to fetch collection data: error=%d\n",
               static_cast<int>(fetch_result.error()));
        return FAIL_RESULT;
    }

    std::expected<http::HttpResponse, http::HttpsParseError> response_parse_result =
        http::parse_response(http_buffer);
    if (!response_parse_result.has_value()) {
        printf("Failed to parse collection data: error=%d\n",
               static_cast<int>(response_parse_result.error()));
        return FAIL_RESULT;
    }

    auto response = *response_parse_result;

    if (response.status_code != 200) {
        printf("Failed to parse collection data: received non-200 status code: %u\n",
               response.status_code);
        return FAIL_RESULT;
    }

    if (response.content_type != "application/json") {
        std::string_view content_type = response.content_type.value_or(std::string_view("nullopt"));
        printf("Failed to parse collection data: non 'application/json' Content-Type: '%.*s'\n",
               static_cast<int>(content_type.size()), content_type.data());
        return FAIL_RESULT;
    }

    if (response.content_length > http_buffer.size()) {
        // The actual buffer overflow is guarded against in tls_client.c, however we should still
        // check the header to detect that the buffer does not contain the complete response.
        printf("Failed to parse collection data: Content-Length of %d exceeds buffer size of %d\n",
               static_cast<int>(response.content_length), static_cast<int>(http_buffer.size()));
        return FAIL_RESULT;
    }

    std::expected<parsing::BinCollectionPair, parsing::ParseError> parse_result =
        parsing::parse_response(response.body);

    if (!parse_result.has_value()) {
        printf("Failed to parse collection data: error=%d\n",
               static_cast<int>(parse_result.error()));
        return FAIL_RESULT;
    }

    auto next_collection = std::get<0>(*parse_result);

    printf("Next bin collection is %d on %04u-%02u-%02u\n",
           static_cast<int>(next_collection.collection_type), next_collection.date.year,
           next_collection.date.month, next_collection.date.day);

    return {.sleep_time_ms = SUCCESS_SLEEP_MS, .next_collection = next_collection};
}

} // namespace worker