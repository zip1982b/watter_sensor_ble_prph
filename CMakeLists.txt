# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(watter_sensor_ble_prph)

target_sources(app PRIVATE src/main.c)

#custom files and folders
target_sources(app PRIVATE src/remote_service/remote.c)
target_sources(app PRIVATE src/battery_voltage/battery_voltage.c)

zephyr_library_include_directories(src/remote_service)
zephyr_library_include_directories(src/battery_voltage)