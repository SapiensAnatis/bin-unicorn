
set(SOURCES 
    display/display.cpp
    http/tls_client.c
    http/http.cpp
    main.cpp
    parsing/arena.cpp
    parsing/parsing.cpp
    worker/worker.cpp
)

set(HEADERS
    display/display.hpp
    http/tls_client.h
    http/http.hpp
    mbedtls_config.h
    parsing/arena.hpp
    parsing/parsing.hpp
    util.hpp
    worker/worker.hpp
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
    http/tls_client.c 
    PROPERTIES
    COMPILE_OPTIONS
    "-Wno-pointer-sign" # unsigned char shenanigans
)

set_target_properties(bin_unicorn PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(bin_unicorn 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mbedtls
    pico_mbedtls
    pico_stdlib
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(bin_unicorn)

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

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# See stdout with the debug probe
pico_enable_stdio_rtt(bin_unicorn 1)