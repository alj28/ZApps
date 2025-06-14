#ifndef _AUDIO_CONF_H_
#define _AUDIO_CONF_H_

#define SAMPLE_FREQUENCY        (CONFIG_SAMPLE_FREQ)
#define SAMPLE_BYTE_WIDTH       (sizeof(uint16_t))
#define SAMPLE_BIT_WIDTH        (8U * SAMPLE_BYTE_WIDTH)
#define NUMBER_OF_CHANNELS      (2)
#define BYTES_PER_SAMPLE        (SAMPLE_BYTE_WIDTH * NUMBER_OF_CHANNELS)
#define SAMPLES_PER_SOF         (DIV_ROUND_UP(SAMPLE_FREQUENCY, 1000))
#define BYTES_PER_SOF           (SAMPLES_PER_SOF * BYTES_PER_SAMPLE)

struct audio_static_cfg {
    uint32_t sample_frequency;
    uint32_t samples_per_sof;
    uint16_t sample_bytes_width;
    uint16_t number_of_channels;
    uint16_t bytes_per_sample;
    uint16_t bytes_per_sof;
};

const struct audio_static_cfg* audio_conf_static_get(void);


#endif