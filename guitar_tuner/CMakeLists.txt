cmake_minimum_required(VERSION 3.20.0)

#set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/../modules/....")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio_card_2)

target_sources(app
    PRIVATE
        src/main.c
        src/blink.c
        src/usb.c
        src/mic.c
        src/signal_buffer.c
        src/board_cli.c
        #src/cic.c
        #src/mic_filter.c
        src/pdm_to_pcm/cic.c
        src/pdm_to_pcm/decimator.c
        src/pdm_to_pcm/mic_filter.c
)

target_include_directories(app
    PRIVATE
        ./src
        ./src/pdm_to_pcm
)
