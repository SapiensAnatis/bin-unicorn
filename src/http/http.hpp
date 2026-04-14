#ifndef HTTP_HTTP_HPP
#define HTTP_HTTP_HPP

#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <variant>

namespace bin_unicorn {

enum class HttpsGetError : int8_t {
    FailedToFormatRequest = -1,
    TlsClientError = -2,
};

enum class HttpsParseError : int8_t {
    Failure = -1,
    UnsupportedResponse = -2,
};

struct HttpResponse {
    uint16_t status_code;
    uint16_t content_length;
    std::optional<std::string_view> content_type;
    std::optional<std::chrono::year_month_day> server_date;
    std::string_view body;
};

/// @brief Fetch data on waste collection from Reading Borough Council.
/// @param buffer The buffer that the UTF-8 response should be written to. Will be demoted to a
/// pointer in tls_client.c; ensure it has enough space reserved.
/// @return Nothing, or a @ref HttpsGetError if parsing failed.
std::expected<std::monostate, HttpsGetError> fetch_collection_data(std::span<char> &buffer);

/// @brief Parse a returned HTTP response buffer to retrieve the status code and some useful
/// headers.
/// @param buffer The buffer that the UTF-8 response was written to.
/// @return A @ref HttpResponse, or a @ref HttpsParseError error if parsing failed.
std::expected<HttpResponse, HttpsParseError> parse_http_response(const std::span<char> &buffer);

} // namespace bin_unicorn

#endif // HTTP_HTTP_HPP