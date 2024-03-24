cmake_minimum_required(VERSION 3.10)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Project name
project(miicare)

# Set the cross-compilation toolchain file (adjust path if necessary)
set(CMAKE_TOOLCHAIN_FILE ./raspberrypi-toolchain.cmake)

#Add includes libraries
include_directories($ENV{HOME}/miicare/third_party/include)

# Find your C++ compiler
#set(CMAKE_CXX_COMPILER $ENV{HOME}/x-tools/armv8-rpi3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)

# Specify the sources
#add_executable(hello_world src/main.cpp)
add_executable(blu_scan src/blu_scan.c)
add_executable(ble_scan src/ble_scan.c)
#link_directories($ENV{HOME}/x-tools/arm-linux-gnueabihf/arm-linux-gnueabihf/lib)

add_library(config_reader STATIC $ENV{HOME}/miicare/utils/config_reader/src/config_reader.c)

set(BLUETOOTH $ENV{HOME}/miicare/third_party/lib/libbluetooth.so.3.19.3)
set(JSON-C $ENV{HOME}/miicare/third_party/lib/libjson-c.so.5.1.0)

target_link_libraries(blu_scan ${BLUETOOTH} ${JSON-C} config_reader)
target_link_libraries(ble_scan ${BLUETOOTH} ${JSON-C})

# Finding the bluetooth lib to link it
# find_library(BLUETOOTH_LIBRARY bluetooth)
# if(BLUETOOTH_LIBRARY)
#     target_link_libraries(blu_scan ${BLUETOOTH_LIBRARY})
# else()
#     message(FATAL_ERROR "Bluetooth library not found")
# endif()

