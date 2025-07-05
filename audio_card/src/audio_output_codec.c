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
#define PRIORITY 0

LOG_MODULE_REGISTER(audio_output_codec, LOG_LEVEL_INF);

const struct device *codec_control = DEVICE_DT_GET(DT_NODELABEL(audio_codec));
const struct device *codec_stream = DEVICE_DT_GET(DT_ALIAS(i2s_codec_tx));


K_MEM_SLAB_DEFINE_STATIC(audio_output_codec_slab, AUDIO_STREAM_CHUNK_SIZE, 4, 4);

static bool init_done = false;

bool is_audio_codec_setup(void)
{
    return init_done;
}

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
        .timeout = 0
    };
    err = i2s_configure(codec_stream, I2S_DIR_TX, &config);
    if (0 != err) {
        LOG_ERR("Cannot configure i2s device...");
    }

    if (0 == err) {
        LOG_INF("Audio output codec ready.");
        init_done = true;
    }
    return err;
}

SYS_INIT_NAMED(
    audio_codec,
    init,
    APPLICATION,
    80
);

static volatile uint32_t thread_count = 0;
static volatile uint32_t thread_step = 0;
static volatile bool print_content = false;
static void thread(void *p1, void *p2, void *p3)
{
    int ret;
    bool started = false;

    while (1) {

        k_sleep(K_MSEC(100));

        if (init_done) {
            break;
        }
    }

    while(1) {
        if (2 < audio_stream_pending_chunk_count()) {
            break;
        }
        k_sleep(K_MSEC(1));
    }

    while (1) {
        thread_step = 0;
        struct audio_chunk* audio_chunk = audio_stream_chunk_get(K_FOREVER);
        thread_step = 1;
        void* pmem = NULL;
        size_t audio_chunk_size = audio_chunk->size;
        ret = k_mem_slab_alloc(
            &audio_output_codec_slab,
            &pmem,
            K_FOREVER
        );
        thread_step = 2;
        memcpy(pmem, (void*)audio_chunk->mem, audio_chunk->size);
        audio_stream_chunk_release(audio_chunk);
        thread_step = 3;
        if (print_content) {
            print_content = false;
            int16_t* p = pmem;
            LOG_INF("Size: %d", audio_chunk_size);
            for (size_t i = 0; i < 16; i++) {
                LOG_INF("%d, %d", p[2*i], p[2*i + 1]);
            }
        }
        ret = i2s_write(codec_stream, pmem, audio_chunk_size);
        if (0 != ret) {
            //LOG_WRN("Fixing I2S state... (err %d)", ret);
            i2s_trigger(codec_stream, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
            started = false;
            ret = i2s_write(codec_stream, pmem, audio_chunk_size);
            if (0 != ret) {
                k_mem_slab_free(
                    &audio_output_codec_slab,
                    pmem
                );
                LOG_ERR("Cannot write to i2s. (err %d)", ret);
                started = false;
                i2s_trigger(codec_stream, I2S_DIR_TX, I2S_TRIGGER_PREPARE);
            }
            //while(1) {
            //    if (2 < audio_stream_pending_chunk_count()) {
            //        break;
            //    }
            //    k_sleep(K_MSEC(1));
            //}

        }
        thread_step = 4;

        if (!started && 2 < k_mem_slab_num_used_get(&audio_output_codec_slab)) {
            thread_step = 5;
            ret = i2s_trigger(codec_stream, I2S_DIR_TX, I2S_TRIGGER_START);
            if (0 == ret) {
                //LOG_INF("I2S started...");
                started = true;
            }
        }
        thread_step = 6;

        thread_count++;
    }
}

K_THREAD_DEFINE(audio_output_codec_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);


#include <zephyr/shell/shell.h>
static int cmd_get_thread_count(const struct shell *sh, size_t argc, char **argv)
{
    shell_info(sh, "Audio output codec thread count: %d, Thread step: %d", thread_count, thread_step);
    size_t free_blocks = k_mem_slab_num_free_get(&audio_output_codec_slab);
    shell_info(sh, "N available blocks: %d", free_blocks);
    print_content = true;
    return 0;
}

SHELL_CMD_REGISTER(tc_aoc, NULL, "TC", cmd_get_thread_count);

