# pico-projects

Monorepository of personal projects for the Raspberry Pi Pico / Pico W.

## Setup

Requires the [Pico SDK](https://github.com/raspberrypi/pico-sdk) and defining `PICO_SDK_PATH`. Some targets need the [Pimoroni Pico libraries](https://github.com/pimoroni/pimoroni-pico).

Also requires an installation of the [GCC ARM toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Download this and set the `bin` folder to be on your `PATH` and the Pico SDK will find the right compiler to use.

Per-user build values (WiFi credentials, project-specific secrets like `BIN_UNICORN_HOME_ADDRESS`) are sourced from environment variables.

### Configuring per-user build values in VS Code

Open **Preferences: Open User Settings (JSON)** from the command palette and add:

```json
"cmake.configureEnvironment": {
    "WIFI_SSID": "Your Network",
    "WIFI_PASSWORD": "Your Password",
    "BIN_UNICORN_HOME_ADDRESS": "40 Caversham Road Reading, RG17EB"
}
```

Notes:
- This goes in **user** settings, not the committed `.vscode/settings.json`.
- After editing user settings, pick the `Debug` preset in the CMake Tools status bar and reconfigure.