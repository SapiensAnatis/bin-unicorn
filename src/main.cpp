#ifndef PICO_W
#error This source file can only be compiled for a Raspberry Pi Pico W.
#endif

#include "worker/worker.hpp"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <span>

static constexpr uint32_t WIFI_CONNECT_FAIL_SLEEP = 5 * 1000;

static constexpr size_t RESPONSE_BUFFER_SIZE = 2048;

/// @brief Connect to the WiFi network using the WIFI_SSID and WIFI_PASSWORD definitions.
/// @return True if successful, otherwise false.
bool connect_wifi() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        printf("Failed to initialise WiFi connection.\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(BIN_UNICORN_WIFI_SSID, BIN_UNICORN_WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to WiFi.\n");
        return false;
    } else {
        printf("Connected to WiFi.\n");
        return true;
    }
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

    char *response_buffer_ptr = (char *)malloc(RESPONSE_BUFFER_SIZE);
    if (response_buffer_ptr == nullptr) {
        fprintf(stderr, "Failed to allocate response buffer\n");
        return 1;
    }

    std::span<char> response_buffer(response_buffer_ptr, RESPONSE_BUFFER_SIZE);

    while (true) {
        auto [sleep, result_option] = worker::do_work_loop(response_buffer);

        bool success = result_option.has_value();

        if (success) {
            // TODO: If the device is started in the day prior to the collection data changing,
            // sleeping here could lead to stale data being displayed. Consider using NTP instead to
            // re-run the work loop at a specific time when an update is expected.

            printf("Work loop succeeded. Sleeping for %lu ms\n", static_cast<unsigned long>(sleep));
            sleep_ms(sleep);
        } else {
            printf("Work loop failed! Sleeping for %lu ms\n", static_cast<unsigned long>(sleep));
            sleep_ms(sleep);
        }
    }

    cyw43_arch_deinit();
    free(response_buffer_ptr);

    return 0;
}