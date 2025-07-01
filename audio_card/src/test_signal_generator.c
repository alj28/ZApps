
#include <stdint.h>
#include <zephyr/kernel.h>

#include "test_signal_generator.h"
#include "audio_conf.h"
#include "arm_math.h"

#define SG_SIGNAL_FREQUENCY     (500)
#define SG_ANGLE_INC_Q31(_f)        (int32_t)(((float)(0x1UL << 31) * 2.0f * 3.14f * (float)(_f)) / 3.14f / (float)(SAMPLE_FREQUENCY))

static int32_t current_angle = 0;
static int32_t angle_inc = SG_ANGLE_INC_Q31(SG_SIGNAL_FREQUENCY);

void generate_signal_section(int16_t* const mem, size_t size)
{
    size_t n_samples = size;

    for (size_t sample_c = 0; sample_c < n_samples; sample_c++) {
        size_t index = 2*sample_c;
        int32_t l = 0L, r = 0L;
        arm_sin_cos_q31(current_angle, &l, &r);
        //mem[index] = (l >> 16);
        //mem[index + 1] = (r >> 16);
        mem[index + 1] = (l >> 16);
        mem[index] = (r >> 16);
        current_angle += angle_inc;
    }
}


#include <zephyr/shell/shell.h>
static int cmd_set_test_signal_frequency(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_error(sh, "Invalid number of arguments...");
        return -1;
    }

    int32_t fs = atoi(argv[1]);
    if (fs < 50) {
        shell_error(sh, "Frequency %d is too low (limit %d Hz).", fs, 50);
        return -1;
    } else if (fs > SAMPLE_FREQUENCY) {
        shell_error(sh, "Frequency %d is too high. (limit %d Hz)", fs, SAMPLE_FREQUENCY);
        return -1;
    }

    angle_inc = SG_ANGLE_INC_Q31(fs);
    shell_print(sh, "OK");
    return 0;
}

SHELL_CMD_REGISTER(sf, NULL, "Signal freqeuncy...", cmd_set_test_signal_frequency);