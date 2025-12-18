cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(lib/pico-sdk/external/pico_sdk_import.cmake)

project(orgnet)

pico_sdk_init()

file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp
)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(orgnet ${SOURCES})

target_link_libraries(orgnet
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(orgnet 1)
pico_enable_stdio_uart(orgnet 0)

pico_add_extra_outputs(orgnet)