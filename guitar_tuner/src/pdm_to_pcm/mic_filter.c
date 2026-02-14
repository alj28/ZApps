#include <string.h>
#include <stdlib.h>

#include "mic_filter.h"
#include "cic.h"
#include "arm_math.h"
#include "decimator.h"

/*-------------------------------------------------------------
        CIC FILTER
  -----------------------------------------------------------*/
#define MIC_FILTER_CONF_CIC_LOG2_DECIMATION     (4)
#define MIC_FILTER_CONF_CIC_OUTPUT_GAIN         (-4)

struct cic cic_stage;

static int32_t cic_stage_init(void)
{
    return cic_init(
        &cic_stage,
        MIC_FILTER_CONF_CIC_LOG2_DECIMATION,
        MIC_FILTER_CONF_CIC_OUTPUT_GAIN
    );
}

static int32_t cic_stage_push(uint32_t sample)
{
    int32_t is_new_sample_available = cic_push(&cic_stage, sample);
    return is_new_sample_available;
}

static int16_t cic_stage_get_last_sample(void)
{
    return cic_stage.last_output;
}

/*-------------------------------------------------------------
        HALF PASS FILTER + DECIMATOR 2
  -----------------------------------------------------------*/
#define DECIMATOR_2_FILTER_LEN  24
#define DECIMATOR_2_FACTOR      2
static q15_t decimator_2_taps[DECIMATOR_2_FILTER_LEN] = {
    6,
    74,
    260,
    464,
    351,
    -367,
    -1357,
    -1473,
    491,
    4474,
    8614,
    10385,
    8614,
    4474,
    491,
    -1473,
    -1357,
    -367,
    351,
    464,
    260,
    74,
    6,
    0,
};

static q15_t decimator_2_states[DECIMATOR_2_FILTER_LEN + DECIMATOR_2_FACTOR - 1] = {0};
static q15_t decimator_2_inputs[DECIMATOR_2_FACTOR] = {0};
struct decimator decimator_2_filter;

static int32_t decimator_2_init(void)
{
    return decimator_init(
        &decimator_2_filter,
        decimator_2_taps,
        decimator_2_states,
        DECIMATOR_2_FILTER_LEN,
        decimator_2_inputs,
        DECIMATOR_2_FACTOR
    );
}

static int32_t decimator_2_push(int16_t sample)
{
    return decimator_push(
        &decimator_2_filter,
        sample
    );
}

static int16_t decimator_2_get_last_output(void)
{
    return decimator_get_last_output(&decimator_2_filter);
}

/*-------------------------------------------------------------
        HALF PASS FILTER + DECIMATOR 3
  -----------------------------------------------------------*/
#define HALF_PASS_FILTER_COEFF  2

#if 1 == HALF_PASS_FILTER_COEFF
#define DECIMATOR_3_FILTER_LEN  60
#define DECIMATOR_3_FACTOR      2
static q15_t decimator_3_taps[DECIMATOR_3_FILTER_LEN] = {
    -160,
    88,
    251,
    145,
    -114,
    -96,
    183,
    196,
    -160,
    -267,
    143,
    367,
    -87,
    -471,
    -1,
    582,
    138,
    -692,
    -336,
    798,
    621,
    -894,
    -1047,
    974,
    1756,
    -1035,
    -3273,
    1073,
    10362,
    15298,
    10362,
    1073,
    -3273,
    -1035,
    1756,
    974,
    -1047,
    -894,
    621,
    798,
    -336,
    -692,
    138,
    582,
    -1,
    -471,
    -87,
    367,
    143,
    -267,
    -160,
    196,
    183,
    -96,
    -114,
    145,
    251,
    88,
    -160,
    0,
};
#elif 2 == HALF_PASS_FILTER_COEFF
#define DECIMATOR_3_FILTER_LEN  28
#define DECIMATOR_3_FACTOR      2
static q15_t decimator_3_taps[DECIMATOR_3_FILTER_LEN] = {
    -252,
    60,
    599,
    625,
    -240,
    -872,
    1,
    1410,
    824,
    -1856,
    -2614,
    2187,
    10125,
    14080,
    10125,
    2187,
    -2614,
    -1856,
    824,
    1410,
    1,
    -872,
    -240,
    625,
    599,
    60,
    -252,
    0,
};
#endif

static q15_t decimator_3_states[DECIMATOR_3_FILTER_LEN + DECIMATOR_3_FACTOR - 1] = {0};
static q15_t decimator_3_inputs[DECIMATOR_3_FACTOR] = {0};
struct decimator decimator_3_filter;

static int32_t decimator_3_init(void)
{
    return decimator_init(
        &decimator_3_filter,
        decimator_3_taps,
        decimator_3_states,
        DECIMATOR_3_FILTER_LEN,
        decimator_3_inputs,
        DECIMATOR_3_FACTOR
    );
}

static int32_t decimator_3_push(int16_t sample)
{
    return decimator_push(
        &decimator_3_filter,
        sample
    );
}

static int16_t decimator_3_get_last_output(void)
{
    return decimator_get_last_output(&decimator_3_filter);
}

/*-------------------------------------------------------------
        DC REMOVAL
  -----------------------------------------------------------*/

static q15_t dc_removal_prev_input = 0;
static q15_t dc_removal_last_output = 0;

static int32_t dc_removal_init(void)
{
    dc_removal_prev_input = 0;
    dc_removal_last_output = 0;
    return 0;
}

int32_t dc_removal_push(q15_t sample)
{
    int32_t tmp_1 = sample - dc_removal_prev_input;
    int32_t tmp_2 = ((32700 * dc_removal_last_output) >> 15);
    dc_removal_last_output = tmp_1 + tmp_2;
    dc_removal_prev_input = sample;
    return 1;
}

static int16_t dc_removal_get_last_output(void)
{
    return dc_removal_last_output;
}


int32_t mic_filter_init(void)
{
    int32_t rv = cic_stage_init();
    if (0 != rv) return rv;
    
    rv = decimator_2_init();
    if (0 != rv) return rv;

    rv = decimator_3_init();
    if (0 != rv) return rv;

    rv = dc_removal_init();
    if (0 != rv) return rv;

    return 0;
}

int32_t mic_filter_push(uint32_t sample)
{
    int32_t is_new_sample_available = cic_stage_push(sample);
    if (0 == is_new_sample_available) return 0;
    
    int16_t cic_output = cic_stage_get_last_sample();
    is_new_sample_available = decimator_2_push(cic_output);
    if (0 == is_new_sample_available) return 0;

    int16_t decimator_2_output = decimator_2_get_last_output();
    is_new_sample_available = decimator_3_push(decimator_2_output);
    if (0 == is_new_sample_available) return 0;

    int16_t decimator_3_output = decimator_3_get_last_output();
    is_new_sample_available = dc_removal_push(decimator_3_output);
    
    return is_new_sample_available;
}

int16_t mic_filter_get_last_output(void)
{
    return dc_removal_get_last_output();
}

int32_t mic_filter_get_debug_sig(uint32_t sig)
{
    if (eMIC_FILTER_DEBUG_SIG_CIC_OUTPUT == sig) return cic_stage_get_last_sample();
    else if (eMIC_FILTER_DEBUG_SIG_DECIMATOR_2_OUTPUT == sig) return decimator_2_get_last_output();
    else if (eMIC_FILTER_DEBUG_SIG_DECIMATOR_3_OUTPUT == sig) return decimator_3_get_last_output();
    else if (eMIC_FILTER_DEBUG_SIG_DC_REMOVAL_OUTPUT == sig) return dc_removal_get_last_output();
    return 0;
}


