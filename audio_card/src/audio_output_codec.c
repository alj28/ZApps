#include <stdlib.h>
#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/audio/codec.h>

#include "audio_output_codec.h"
#include "audio_conf.h"
#include "test_signal_generator.h"
#include "audio_stream.h"

#include "arm_math.h"

#define STACKSIZE 1024
#define PRIORITY 5

LOG_MODULE_REGISTER(audio_output_codec, LOG_LEVEL_INF);

const struct device *codec_control = DEVICE_DT_GET(DT_NODELABEL(audio_codec));
const struct device *codec_stream = DEVICE_DT_GET(DT_ALIAS(i2s_codec_tx));


K_MEM_SLAB_DEFINE_STATIC(audio_output_codec_slab, AUDIO_STREAM_CHUNK_SIZE, 4, 4);

static int init(void)
{
    int err = 0;
    if (!device_is_ready(codec_control)) {
        LOG_ERR("%s is not ready...", codec_control->name);
        err = -1;
    }
    if (!device_is_ready(codec_stream)) {
        LOG_ERR("%s is not ready...", codec_stream->name);
        err = -1;
    }

    struct audio_codec_cfg audio_cfg = {
        .dai_route = AUDIO_ROUTE_PLAYBACK,
        .dai_type = AUDIO_DAI_TYPE_I2S,
        .dai_cfg.i2s.word_size = SAMPLE_BIT_WIDTH,
        .dai_cfg.i2s.channels = NUMBER_OF_CHANNELS,
        .dai_cfg.i2s.format = I2S_FMT_DATA_FORMAT_I2S,
        .dai_cfg.i2s.options = I2S_OPT_FRAME_CLK_MASTER,
        .dai_cfg.i2s.frame_clk_freq = SAMPLE_FREQUENCY,
        .dai_cfg.i2s.mem_slab = &audio_output_codec_slab,
        .dai_cfg.i2s.block_size = BYTES_PER_SOF
    };
    err = audio_codec_configure(codec_control, &audio_cfg);
    if (0 != err) {
        LOG_ERR("Cannot configure audio codec...");
    }

    struct i2s_config config = {
        .word_size = SAMPLE_BIT_WIDTH,
        .channels = NUMBER_OF_CHANNELS,
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER,
        .frame_clk_freq = SAMPLE_FREQUENCY,
        .mem_slab = &audio_output_codec_slab,
        .block_size = BYTES_PER_SOF,
        .timeout = 2000UL
    };
    err = i2s_configure(codec_stream, I2S_DIR_TX, &config);
    if (0 != err) {
        LOG_ERR("Cannot configure i2s device...");
    }

    if (0 == err) {
        LOG_INF("Audio output codec ready.");
    }
    return err;
}

SYS_INIT_NAMED(
    audio_codec,
    init,
    APPLICATION,
    1
);

static void thread(void *p1, void *p2, void *p3)
{
    int ret;
    bool started = false;

    while (1) {
        struct audio_chunk* audio_chunk = audio_stream_chunk_get(K_FOREVER);
        void* pmem = NULL;
        size_t audio_chunk_size = audio_chunk->size;
        k_mem_slab_alloc(
            &audio_output_codec_slab,
            &pmem,
            K_FOREVER
        );
        memcpy(pmem, (void*)audio_chunk->mem, audio_chunk->size);
        audio_stream_chunk_release(audio_chunk);
        i2s_write(codec_stream, pmem, audio_chunk_size);

        if (!started) {
            i2s_trigger(codec_stream, I2S_DIR_TX, I2S_TRIGGER_START);
            LOG_INF("I2S started...");
            started = true;
        }
    }
}

K_THREAD_DEFINE(audio_output_codec_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);

