add_library(cjson STATIC ${CMAKE_SOURCE_DIR}/lib/cJSON/cJSON.c ${CMAKE_SOURCE_DIR}/lib/cJSON/cJSON.h)
target_include_directories(cjson PUBLIC SYSTEM ${CMAKE_SOURCE_DIR}/lib/cJSON)

set(SOURCES 
    http/tls_client.c
    http/http.cpp
    parsing/parsing.cpp
    main.cpp
)

set(HEADERS    
    http/tls_client.h
    http/http.hpp
    parsing/parsing.hpp
    mbedtls_config.h
    util.hpp
)

add_executable(bin_unicorn)

target_sources(bin_unicorn 
    PRIVATE ${SOURCES}
    PUBLIC
    FILE_SET HEADERS
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

target_compile_options(bin_unicorn PRIVATE
    -Wall
    -Werror
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

# Disable Pico SDK warnings.
# Bit of a hack, but not all headers appear to be included using -isystem so -Werror fails the build without this
file(GLOB_RECURSE PICO_SDK_FILES "${PICO_SDK_PATH}/*.c" "${PICO_SDK_PATH}/*.h")

set_source_files_properties(
    ${PICO_SDK_FILES}
    PROPERTIES
    COMPILE_FLAGS
    -w # disable all warnings
)

# This file has to do some cursed C stuff
set_source_files_properties(
    http/tls_client.c 
    PROPERTIES
    COMPILE_FLAGS
    -Wno-pointer-sign # unsigned char shenanigans
)

target_link_libraries(bin_unicorn 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
    pico_stdlib
    hardware_pio
    hardware_dma
    cjson
)

pico_add_extra_outputs(bin_unicorn)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")