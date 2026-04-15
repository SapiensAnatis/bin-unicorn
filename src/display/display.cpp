#include "display/display.hpp"

#include "pico/stdlib.h"

#ifndef RASPBERRYPI_PICO2_W
#error This source file can only be compiled for a Raspberry Pi Pico 2 W.
#endif

namespace bin_unicorn {

/// @brief
static constexpr uint WIFI_CONNECTED_LED_GPIO = 10; ///< Pin attached to a diffused blue LED.
static constexpr uint ERROR_LED_GPIO = 11;          ///< Pin attached to a diffused red LED.

static constexpr uint FOOD_WASTE_BIN_LED_GPIO = 18; ///< Pin attached to a blue LED.
static constexpr uint GENERAL_BIN_LED_GPIO = 19;    ///< Pin attached to a white LED.
static constexpr uint RECYCLING_BIN_LED_GPIO = 21;  ///< Pin attached to a red LED.
static constexpr uint GARDEN_BIN_LED_GPIO = 20;     ///< Pin attached to a green LED.

void display_init() {
    gpio_init(WIFI_CONNECTED_LED_GPIO);
    gpio_set_dir(WIFI_CONNECTED_LED_GPIO, GPIO_OUT);
    gpio_put(WIFI_CONNECTED_LED_GPIO, false);

    gpio_init(ERROR_LED_GPIO);
    gpio_set_dir(ERROR_LED_GPIO, GPIO_OUT);
    gpio_put(ERROR_LED_GPIO, false);

    gpio_init(RECYCLING_BIN_LED_GPIO);
    gpio_set_dir(RECYCLING_BIN_LED_GPIO, GPIO_OUT);
    gpio_put(RECYCLING_BIN_LED_GPIO, false);

    gpio_init(GARDEN_BIN_LED_GPIO);
    gpio_set_dir(GARDEN_BIN_LED_GPIO, GPIO_OUT);
    gpio_put(GARDEN_BIN_LED_GPIO, false);

    gpio_init(GENERAL_BIN_LED_GPIO);
    gpio_set_dir(GENERAL_BIN_LED_GPIO, GPIO_OUT);
    gpio_put(GENERAL_BIN_LED_GPIO, false);

    gpio_init(FOOD_WASTE_BIN_LED_GPIO);
    gpio_set_dir(FOOD_WASTE_BIN_LED_GPIO, GPIO_OUT);
    gpio_put(FOOD_WASTE_BIN_LED_GPIO, false);
}

void test_leds() {
    gpio_put(WIFI_CONNECTED_LED_GPIO, true);
    gpio_put(ERROR_LED_GPIO, true);
    gpio_put(RECYCLING_BIN_LED_GPIO, true);
    gpio_put(GARDEN_BIN_LED_GPIO, true);
    gpio_put(GENERAL_BIN_LED_GPIO, true);
    gpio_put(FOOD_WASTE_BIN_LED_GPIO, true);
}

void display_wifi_connected() { gpio_put(WIFI_CONNECTED_LED_GPIO, true); }

void display_error() { gpio_put(ERROR_LED_GPIO, true); }

void clear_error() { gpio_put(ERROR_LED_GPIO, false); }

void clear_all_bin_leds() {
    gpio_put(RECYCLING_BIN_LED_GPIO, false);
    gpio_put(GARDEN_BIN_LED_GPIO, false);
    gpio_put(GENERAL_BIN_LED_GPIO, false);
    gpio_put(FOOD_WASTE_BIN_LED_GPIO, false);
}

void display_food_waste() { gpio_put(FOOD_WASTE_BIN_LED_GPIO, true); }

void display_recycling() { gpio_put(RECYCLING_BIN_LED_GPIO, true); }

void display_garden_waste() { gpio_put(GARDEN_BIN_LED_GPIO, true); }

void display_domestic_waste() { gpio_put(GENERAL_BIN_LED_GPIO, true); }

} // namespace bin_unicorn