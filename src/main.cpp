#include "display/display.hpp"
#include "parsing/parsing.hpp"
#include "worker/worker.hpp"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <span>

#ifndef RASPBERRYPI_PICO2_W
#error This source file can only be compiled for a Raspberry Pi Pico 2 W.
#endif

static constexpr uint32_t WIFI_CONNECT_FAIL_SLEEP = 5 * 1000;

/// @brief Connect to the WiFi network using the WIFI_SSID and WIFI_PASSWORD definitions.
/// @return True if successful, otherwise false.
static bool connect_wifi() {
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

static void display_collection(bin_unicorn::CollectionType type) {
    using enum bin_unicorn::CollectionType;

    switch (type) {
    case DomesticWaste:
        bin_unicorn::display_domestic_waste();
        break;
    case FoodWaste:
        bin_unicorn::display_food_waste();
        break;
    case Recycling:
        bin_unicorn::display_recycling();
        break;
    case GardenWaste:
        bin_unicorn::display_garden_waste();
        break;
    default:
        fprintf(stderr, "Cannot display unknown collection type: %d", static_cast<int>(type));
        bin_unicorn::display_error();
        break;
    }
}

int main() {
    stdio_init_all();

    bin_unicorn::display_init();

    bool connected_to_wifi = false;
    do {
        connected_to_wifi = connect_wifi();

        if (!connected_to_wifi) {
            cyw43_arch_deinit();
            bin_unicorn::display_error();
            sleep_ms(WIFI_CONNECT_FAIL_SLEEP);
        }
    } while (!connected_to_wifi);

    bin_unicorn::clear_error();
    bin_unicorn::display_wifi_connected();

    while (true) {
        bin_unicorn::clear_error();

        auto [success, sleep, next_collections] = bin_unicorn::do_work_loop();

        if (success) {
            // TODO: If the device is started in the day prior to the collection data changing,
            // sleeping here could lead to stale data being displayed. Consider using NTP instead to
            // re-run the work loop at a specific time when an update is expected.
            printf("Work loop succeeded\n");

            const auto &[coll1, coll2] = next_collections;

            display_collection(coll1.collection_type);

            // Sometimes two collections occur on the same day - in which case both should be
            // displayed.
            if (coll1.date == coll2.date) {
                display_collection(coll2.collection_type);
            }
        } else {
            printf("Work loop failed!\n");
            bin_unicorn::display_error();
        }

        printf("Sleeping for %" PRIu32 " ms.\n", sleep);
        sleep_ms(sleep);
    }

    cyw43_arch_deinit();

    return 0;
}