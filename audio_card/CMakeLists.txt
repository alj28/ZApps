cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio_card_2)

target_sources(app
    PRIVATE
        src/audio.c
        src/audio_conf.c
        src/audio_output_codec.c
        #src/audio_output_codec_2.c
        src/audio_source.c
        src/audio_stream.c
        #src/audio_usb.c
        src/blink.c
        src/main.c
        src/shell_cmd.c
        src/test_signal_generator.c
        src/usb.c
)

target_include_directories(app
    PRIVATE
        ./src
)

