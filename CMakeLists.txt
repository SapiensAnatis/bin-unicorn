# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(PICO_BOARD pico2_w CACHE STRING "Board type")
set(PICO_DEOPTIMIZED_DEBUG 1)

if (PICO_BOARD STREQUAL pico_w)
    add_compile_definitions(PICO_W)
endif()

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(bin_unicorn C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
# set(PICO_CXX_ENABLE_EXCEPTIONS 1)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()


set(SOURCES 
    src/display/display.cpp
    src/http/tls_client.c
    src/http/http.cpp
    src/main.cpp
    src/parsing/arena.cpp
    src/parsing/parsing.cpp
    src/worker/worker.cpp
)

set(HEADERS
    src/display/display.hpp
    src/http/tls_client.h
    src/http/http.hpp
    src/mbedtls_config.h
    src/parsing/arena.hpp
    src/parsing/parsing.hpp
    src/util.hpp
    src/worker/worker.hpp
)

add_executable(bin_unicorn)

target_sources(bin_unicorn 
    PRIVATE ${SOURCES}
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src/ # Automatically adds src/ to target_include_directories
    FILES ${HEADERS}
)

# We can't set these variables using $env{} in CMakePresets.json due to a bug in the CMake Tools extension
# https://github.com/microsoft/vscode-cmake-tools/issues/4384
foreach(var BIN_UNICORN_WIFI_SSID BIN_UNICORN_WIFI_PASSWORD BIN_UNICORN_HOME_ADDRESS)
    if(NOT ${var})
        if(DEFINED ENV{${var}})
            set(${var} "$ENV{${var}}" CACHE STRING "${var}" FORCE)
        else()
            message(FATAL_ERROR "${var} is not set. Export it in your environment "
                                "(e.g. via direnv) or pass -D${var}=... to CMake. "
                                "See README.md for details.")
        endif()
    endif()
endforeach()


target_compile_definitions(bin_unicorn PRIVATE
    BIN_UNICORN_WIFI_SSID=\"${BIN_UNICORN_WIFI_SSID}\"
    BIN_UNICORN_WIFI_PASSWORD=\"${BIN_UNICORN_WIFI_PASSWORD}\"
    BIN_UNICORN_HOME_ADDRESS=\"${BIN_UNICORN_HOME_ADDRESS}\"
)

# Allows analysing binary size with nm --size-sort --print-size ./build/debug/bin/bin_unicorn.elf
# target_link_options(bin_unicorn PRIVATE "-Wl,-Map=out.map")

# The Pico SDK brings in its source files using INTERFACE targets, which means that any options we set in
# target_compile_options will be used to compile the SDK as well. This causes the build to fail if our settings
# are too strict. Work around this by manually setting the compile flags on the source files we own.
set_source_files_properties(
    ${SOURCES}
    PROPERTIES
    COMPILE_OPTIONS
    "-Wall;-Werror;-Wextra;-Wpedantic;-Wno-unused-parameter"
)

# We can't disable this warning globally as the flag is not supported when compiling C++ files.
set_source_files_properties(
    src/http/tls_client.c 
    PROPERTIES
    COMPILE_OPTIONS
    "-Wno-pointer-sign" # unsigned char shenanigans
)

target_link_libraries(bin_unicorn 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
    pico_stdlib
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(bin_unicorn)
# See stdout with the debug probe
pico_enable_stdio_rtt(bin_unicorn 1)

include(FetchContent)

FetchContent_Declare(
    cJSON
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_SHA        fb16e5cf358798aabb049655975cde8427101056
)

set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_TEST       OFF CACHE BOOL "" FORCE)
set(ENABLE_CJSON_UTILS      OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_AND_STATIC_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cJSON)

target_link_libraries(bin_unicorn cjson)
target_include_directories(bin_unicorn PRIVATE SYSTEM ${cjson_SOURCE_DIR})

