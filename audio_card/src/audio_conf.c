
#include <stdio.h>
#include <zephyr/kernel.h>

#include "audio_conf.h"



static const struct audio_static_cfg audio_static_cfg = {
    .sample_frequency = SAMPLE_FREQUENCY,
    .samples_per_sof = SAMPLES_PER_SOF,
    .sample_bytes_width = SAMPLE_BYTE_WIDTH,
    .number_of_channels = NUMBER_OF_CHANNELS,
    .bytes_per_sample = BYTES_PER_SAMPLE,
    .bytes_per_sof = BYTES_PER_SOF,
};

const struct audio_static_cfg* audio_conf_static_get(void)
{
    return &audio_static_cfg;
}