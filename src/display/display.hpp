#ifndef DISPLAY_DISPLAY_H_
#define DISPLAY_DISPLAY_H_

namespace display {

/// @brief Initialises the relevant GPIO ports for display.
void display_init();
/// @brief Function to test all LEDs are wired up properly by toggling them all on.
[[maybe_unused]] void test_leds();

/// @brief Show that the device has connected to WiFi by lighting up the diffused blue LED.
void display_wifi_connected();
/// @brief Show that an error has occurred by lighting up the diffused red LED.
void display_error();
/// @brief Show that an error has been recovered from by clearing a red LED.
void clear_error();

/// @brief Show that the next collection is food waste by lighting up a blue LED.
void display_food_waste();
/// @brief Show that the next collection is recycling by lighting up a red LED.
void display_recycling();
/// @brief Show that the next collection is garden waste by lighting up a green LED.
void display_garden_waste();
/// @brief Show that the next collection is general waste by lighting up a white LED.
void display_domestic_waste();

} // namespace display

#endif // DISPLAY_DISPLAY_H_