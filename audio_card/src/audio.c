
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/audio/codec.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_audio.h>

#include "audio_conf.h"

LOG_MODULE_REGISTER(audio, LOG_LEVEL_INF);

static void log_info(const struct audio_static_cfg* const cfg)
{
    LOG_INF("Sampling frequency:    %d Hz", cfg->sample_frequency);
    LOG_INF("Sample byte width:     %d bytes", cfg->sample_bytes_width);
    LOG_INF("Number of channels:    %d", cfg->number_of_channels);
    LOG_INF("Bytes per sample:      %d bytes", cfg->bytes_per_sample);
    LOG_INF("Samples per SOF:       %d", cfg->samples_per_sof);
    LOG_INF("Bytes per SOF:         %d bytes", cfg->bytes_per_sof);
}

static int init(void)
{
    log_info(audio_conf_static_get());
    return 0;
}


SYS_INIT_NAMED(
    audio,
    init,
    APPLICATION,
    0
);
 


 