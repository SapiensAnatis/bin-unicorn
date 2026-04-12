#ifndef PICO_W
#error This source file can only be compiled for a Raspberry Pi Pico W.
#endif

#include "util.hpp"
#include "http/http.hpp"
#include "parsing/parsing.hpp"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include <array>
#include <cstdio>
#include <span>

constexpr uint32_t THREE_HOURS_MS = 3 * 60 * 60 * 1000;

constexpr uint32_t ERROR_SLEEP = THREE_HOURS_MS;
constexpr uint32_t SUCCESS_SLEEP = THREE_HOURS_MS * 2;

constexpr uint32_t WIFI_CONNECT_FAIL_SLEEP = 5 * 1000;

constexpr size_t RESPONSE_BUFFER_SIZE = 2048;

/// @brief Connect to the WiFi network using the WIFI_SSID and WIFI_PASSWORD definitions.
/// @return True if successful, otherwise false.
bool connect_wifi() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        printf("Failed to initialise WiFi connection.\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(BIN_UNICORN_WIFI_SSID, BIN_UNICORN_WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
                                           30000)) {
        printf("Failed to connect to WiFi.\n");
        return false;
    } else {
        printf("Connected to WiFi.\n");
        return true;
    }
}

/// @brief Main worker loop.
/// @param address The address to fetch collection data for.
/// @param response_buffer
/// @return True if the loop succeeded, otherwise false.
bool work_loop(const std::string &address, std::span<char> &response_buffer) {
    using namespace std::literals;

    // Zero out buffer to avoid being able to read uninitialized memory via Content-Length attacks
    std::fill(response_buffer.begin(), response_buffer.end(), 0);

    http::HttpsGetResult fetch_result = http::fetch_collection_data(address, response_buffer);
    if (fetch_result != http::HttpsGetResult::Success) {
        printf("Failed to fetch collection data: error=%d\n", static_cast<int>(fetch_result));
        return false;
    }

    std::expected<http::HttpResponse, http::HttpsParseResult> response_parse_result =
        http::parse_response(response_buffer);
    if (!response_parse_result.has_value()) {
        printf("Failed to parse collection data: error=%d\n",
               static_cast<int>(response_parse_result.error()));
        return false;
    }

    auto response = *response_parse_result;

    if (response.status_code != 200) {
        printf("Failed to parse collection data: received non-200 status code: %u\n",
               response.status_code);
        return false;
    }

    if (response.content_type != "application/json") {
        std::string_view content_type = response.content_type.value_or("undefined"sv);
        printf("Failed to parse collection data: non 'application/json' Content-Type: '%.*s'\n",
               static_cast<int>(content_type.size()), content_type.data());
        return false;
    }

    if (response.content_length > response_buffer.size()) {
        // The actual buffer overflow is guarded against in tls_client.c, however we should still
        // check the header to detect that the buffer does not contain the complete response.
        printf("Failed to parse collection data: Content-Length of %d exceeds buffer size of %d\n",
               static_cast<int>(response.content_length),
               static_cast<int>(response_buffer.size()));
        return false;
    }

    std::expected<parsing::BinCollectionPair, parsing::ParseError> parse_result =
        parsing::parse_response(response.body);

    if (!parse_result.has_value()) {
        printf("Failed to parse collection data: error=%d\n",
               static_cast<int>(parse_result.error()));
        return false;
    }

    auto next_collection = std::get<0>(*parse_result);

    printf("Next bin collection is %d on %04u-%02u-%02u\n",
           static_cast<int>(next_collection.collection_type), next_collection.date.year,
           next_collection.date.month, next_collection.date.day);

    return true;
}

int main() {
    stdio_init_all();

    bool connected_to_wifi = false;
    do {
        connected_to_wifi = connect_wifi();

        if (!connected_to_wifi) {
            cyw43_arch_deinit();
            sleep_ms(WIFI_CONNECT_FAIL_SLEEP);
        }
    } while (!connected_to_wifi);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    std::string address = url_encode(BIN_UNICORN_HOME_ADDRESS);

    char *response_buffer_ptr = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (response_buffer_ptr == nullptr) {
        fprintf(stderr, "Failed to allocate response buffer\n");
        return 1;
    }

    std::span<char> response_buffer(response_buffer_ptr, RESPONSE_BUFFER_SIZE);

    while (true) {
        bool success = work_loop(address, response_buffer);
        if (success) {
            // TODO: If the device is started in the day prior to the collection data changing,
            // sleeping here could lead to stale data being displayed. Consider using NTP instead to
            // re-run the work loop at a specific time when an update is expected.

            printf("Work loop succeeded. Sleeping for %lu ms\n",
                   static_cast<unsigned long>(SUCCESS_SLEEP));
            sleep_ms(SUCCESS_SLEEP);
        } else {
            printf("Work loop failed! Sleeping for %lu ms\n",
                   static_cast<unsigned long>(ERROR_SLEEP));
            sleep_ms(ERROR_SLEEP);
        }
    }

    cyw43_arch_deinit();
    delete response_buffer_ptr;

    return 0;
}