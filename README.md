# bin-unicorn

Embedded application that interfaces with the [Reading Borough Council API](https://api.reading.gov.uk/) to fetch information about the next scheduled bin collection and display it.

Targets the Raspberry Pi Pico W (RP2040) and runs on bare metal.

At one point, it used to light up a [Pimoroni Unicorn LED matrix](https://shop.pimoroni.com/products/pico-unicorn-pack?variant=32369501306963) with colours representing the next collection details, but this functionality is being reworked. For now it writes the information to stdout, so it can only be viewed using a debug probe.

## Setup

Requires the [Pico SDK](https://github.com/raspberrypi/pico-sdk) and defining `PICO_SDK_PATH`. Some targets need the [Pimoroni Pico libraries](https://github.com/pimoroni/pimoroni-pico).

Also requires an installation of the [GCC ARM toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Download this and set the `bin` folder to be on your `PATH` and the Pico SDK will find the right compiler to use.

Per-user build values (WiFi credentials, project-specific secrets like `BIN_UNICORN_HOME_ADDRESS`) are sourced from environment variables.

### Configuring per-user build values in VS Code

Open **Preferences: Open User Settings (JSON)** from the command palette and add:

```json
"cmake.configureEnvironment": {
    "BIN_UNICORN_WIFI_SSID": "Your Network",
    "BIN_UNICORN_WIFI_PASSWORD": "Your Password",
    "BIN_UNICORN_HOME_ADDRESS": "40 Caversham Road Reading, RG17EB"
}
```

Notes:
- This goes in **user** settings, not the committed `.vscode/settings.json`.
- After editing user settings, pick the `Debug` preset in the CMake Tools status bar and reconfigure.