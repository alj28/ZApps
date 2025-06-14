cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio_card_2)

target_sources(app
    PRIVATE
        src/audio.c
        src/audio_conf.c
        src/audio_output_codec.c
        src/blink.c
        src/main.c
        src/shell_cmd.c
        src/usb.c
)

target_include_directories(app
    PRIVATE
        ./src
)

