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