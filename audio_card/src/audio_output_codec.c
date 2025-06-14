
#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/audio/codec.h>

#include "audio_output_codec.h"
#include "audio_conf.h"

#include "arm_math.h"

#define STACKSIZE 1024
#define PRIORITY 5

#define SG_SIGNAL_FREQUENCY     (500)
#define SG_ANGLE_INC_Q31        (int32_t)(((float)(0x1UL << 31) * 2.0f * 3.14f * (float)(SG_SIGNAL_FREQUENCY)) / 3.14f / (float)(SAMPLE_FREQUENCY))
static int32_t current_angle = 0;
static const int32_t angle_inc = SG_ANGLE_INC_Q31;
static void generate_signal_section(int16_t* const mem, size_t size);

LOG_MODULE_REGISTER(audio_output_codec, LOG_LEVEL_INF);

const struct device *codec_control = DEVICE_DT_GET(DT_NODELABEL(audio_codec));
const struct device *codec_stream = DEVICE_DT_GET(DT_ALIAS(i2s_codec_tx));


K_MEM_SLAB_DEFINE_STATIC(audio_output_codec_slab, BYTES_PER_SOF, 4, 4);

static int init(void)
{
    LOG_INF("Test signal frequency: %d", SG_SIGNAL_FREQUENCY);
    LOG_INF("Angle increment (Q31): %d", SG_ANGLE_INC_Q31);

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
    void* pmem = NULL;
    bool started = false;
    int ret = k_mem_slab_alloc(
        &audio_output_codec_slab,
        &pmem,
        K_FOREVER
    );
    if (0 > ret) {
        LOG_ERR("Cannot allocate memory slab...");
        return;
    }
    generate_signal_section((int16_t*)pmem, SAMPLES_PER_SOF);

    while(1) {
        ret = k_mem_slab_alloc(
            &audio_output_codec_slab,
            &pmem,
            Z_TIMEOUT_TICKS(2000UL)
        );
        if (0 > ret) {
            LOG_ERR("Cannot allocate memory slab...");
            return;
        }
        generate_signal_section((int16_t*)pmem, SAMPLES_PER_SOF);

        ret = i2s_write(codec_stream, pmem, BYTES_PER_SOF);
        if (0 > ret) {
            LOG_ERR("Cannot write to i2s...");
            return;
        }

        if (!started) {
            i2s_trigger(codec_stream, I2S_DIR_TX, I2S_TRIGGER_START);
            LOG_INF("I2S started...");
            started = true;
        }

    }
}

K_THREAD_DEFINE(audio_output_codec_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);


static void generate_signal_section(int16_t* const mem, size_t size)
{
    size_t n_samples = size;
    //assert(0 == (n_samples % 2));

    for (size_t sample_c = 0; sample_c < n_samples; sample_c++) {
        int32_t l = 0L, r = 0L;
        arm_sin_cos_q31(current_angle, &l, &r);
        mem[sample_c] = (l >> 16);
        mem[sample_c + 1] = (r >> 16);
        current_angle += angle_inc;
    }
}