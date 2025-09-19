cmake_minimum_required(VERSION 3.13)

include(lib/pico-sdk/external/pico_sdk_import.cmake)

project(orgnet)

pico_sdk_init()

file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp
)

add_executable(orgnet ${SOURCES})

target_link_libraries(orgnet pico_stdlib pico_multicore)

pico_enable_stdio_usb(orgnet 1)
pico_enable_stdio_uart(orgnet 0)

pico_add_extra_outputs(orgnet)