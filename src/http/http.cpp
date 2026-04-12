#include "http/http.hpp"

#include "util.hpp"

extern "C" {
#include "http/tls_client.h"
}

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <optional>
#include <span>

namespace http {

// ISRG Root X1
// Expiry: Mon, 04 Jun 2035 11:04:38 GMT
static constexpr uint8_t READING_GOV_UK_ROOT_CERT[] = "-----BEGIN CERTIFICATE-----\n\
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n\
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n\
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n\
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n\
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n\
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n\
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n\
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n\
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n\
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n\
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n\
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n\
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n\
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n\
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n\
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n\
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n\
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n\
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n\
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n\
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n\
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n\
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n\
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n\
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n\
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n\
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n\
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n\
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n\
-----END CERTIFICATE-----\n";

static constexpr const char *READING_GOV_UK_HOST = "api.reading.gov.uk";

static constexpr const char *READING_GOV_UK_HEADERS = "Connection: close\r\n"
                                                      "Host: api.reading.gov.uk\r\n"
                                                      "Accept: application/json\r\n"
                                                      "User-Agent: bin_unicorn/0.1.0 RP2040\r\n"
                                                      "GitHub-Username: sapiensanatis\r\n";

static constexpr const char *HTTP_REQUEST_FORMAT = "GET %s HTTP/1.1\r\n"
                                                   "%s\r\n"
                                                   "\r\n";

static consteval size_t string_length(const std::string_view arg) {
    // Get string length without pesky null terminator which is included in sizeof()
    return arg.size();
}

static void print_failed_to_find_header(const std::string_view header_name) {
    fprintf(stderr, "Failed to find header: %.*s\n", static_cast<int>(header_name.length()),
            header_name.data());
}

static std::optional<std::string_view> find_header_value(const std::string_view headers_string,
                                                         const std::string_view header_name) {

    auto header_start = headers_string.find(header_name);
    if (header_start == std::string_view::npos) {
        print_failed_to_find_header(header_name);
        return std::nullopt;
    }

    auto header_end = headers_string.find("\r\n", header_start);
    if (header_end == std::string_view::npos) {
        print_failed_to_find_header(header_name);
        return std::nullopt;
    }

    // +2 for colon and space before value.
    auto header_value_start = header_start + header_name.length() + 2;

    return std::string_view(headers_string.begin() + header_value_start,
                            headers_string.begin() + header_end);
}

std::expected<std::monostate, HttpsGetError> fetch_collection_data(std::span<char> &buffer) {
    static constexpr auto uri = concat("/rbc/mycollections/", url_encode(BIN_UNICORN_HOME_ADDRESS));

    const size_t bytes_written = snprintf(buffer.data(), buffer.size(), HTTP_REQUEST_FORMAT,
                                          uri.c_str(), READING_GOV_UK_HEADERS);

    if (bytes_written >= buffer.size()) {
        return std::unexpected(HttpsGetError::FailedToFormatRequest);
    }

    TlsClientRequest request = {
        .hostname = READING_GOV_UK_HOST,
        .request = buffer.data(),
        .cert = READING_GOV_UK_ROOT_CERT,
        .cert_len = sizeof(READING_GOV_UK_ROOT_CERT),
    };

    printf("Starting HTTPS GET: https://%s%s\n", READING_GOV_UK_HOST, uri.c_str());

    auto start = std::chrono::high_resolution_clock::now();

    int8_t result = tls_request(request, buffer.data(), buffer.size());

    if (result < 0) {
        printf("Request failed; err=%d\n", static_cast<int>(result));
        return std::unexpected(HttpsGetError::TlsClientError);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("Request completed in %lld ms\n", static_cast<long long>(milliseconds.count()));

    return {};
}

std::expected<HttpResponse, HttpsParseError> parse_response(const std::span<char> &buffer) {
    /* A raw HTTP response looks like:
     *
     *   HTTP/1.1 200 OK
     *   Server: nginx
     *   Date: Tue, 11 Feb 2025 22:17:30 GMT
     *   Content-Type: application/json
     *   Content-Length: 1031
     *   Connection: close
     *   Access-Control-Allow-Origin: https://api.reading.gov.uk
     *   Vary: Origin
     *
     *   {
     *     "Collections": [
     * ... <rest of the response body>
     *
     * Each line is delimeted by \r\n.
     */

    std::string_view buffer_string(buffer.data(), buffer.size());

    auto headers_end = buffer_string.find("\r\n\r\n");
    if (headers_end == std::string_view::npos) {
        fprintf(stderr, "Failed to find end of response headers\n");
        return std::unexpected(HttpsParseError::Failure);
    }

    std::string_view headers_string = buffer_string.substr(0, headers_end);

    auto status_code_start = string_length("HTTP/1.1 ");
    auto status_code_size = 3;

    if (!headers_string.starts_with("HTTP/1.1") ||
        headers_string.length() < string_length("HTTP/1.1 ") + status_code_size) {
        return std::unexpected(HttpsParseError::Failure);
    }

    std::string_view status_code_view(headers_string.begin() + status_code_start, status_code_size);
    uint16_t status_code;
    if (!try_parse_number(status_code_view, status_code)) {
        fprintf(stderr, "Failed to parse status code\n");
        return std::unexpected(HttpsParseError::Failure);
    }

    // We do not support chunked encoding, but the RBC API does not appear to use it. We should
    // check this continues to be the case, however.
    //
    // The header could also be sent as Transfer-Encoding: chunked, gzip which erroneously pass this
    // check, but that shouldn't happen since we don't send Accept-Encoding: gzip.
    if (auto transfer_encoding = find_header_value(headers_string, "Transfer-Encoding");
        transfer_encoding == "chunked") {
        fprintf(stderr, "Chunked encoding is not supported\n");
        return std::unexpected(HttpsParseError::UnsupportedResponse);
    }

    std::optional<std::string_view> content_length_view =
        find_header_value(headers_string, "Content-Length");
    uint16_t content_length;
    if (!content_length_view || !try_parse_number(*content_length_view, content_length)) {
        fprintf(stderr, "Failed to parse Content-Length\n");
        return std::unexpected(HttpsParseError::Failure);
    }

    std::optional<std::string_view> content_type_view =
        find_header_value(headers_string, "Content-Type");

    std::string_view body =
        buffer_string.substr(headers_end + string_length("\r\n\r\n"), content_length);

    return HttpResponse{.status_code = status_code,
                        .content_length = content_length,
                        .content_type = content_type_view,
                        .body = body};
}

} // namespace http