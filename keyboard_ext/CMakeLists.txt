# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(keyboard_ext)

FILE(GLOB app_sources src/*.c)
target_sources(
    app 
    PRIVATE 
    src/app_usb.c
    src/app_inputs.c
    src/app_cli.c
    src/app_keyboard.c
)
target_include_directories(
    app
    PRIVATE
    src
)
