#ifndef DSP_FILTER_H
#define DSP_FILTER_H

#include <stdint.h>

int32_t mic_filter_init(void);
int32_t mic_filter_push(uint32_t sample);
int16_t mic_filter_get_last_output(void);

enum mic_filter_debug_signals
{
    eMIC_FILTER_DEBUG_SIG_CIC_OUTPUT = 0,
    eMIC_FILTER_DEBUG_SIG_DECIMATOR_2_OUTPUT = 1,
    eMIC_FILTER_DEBUG_SIG_DECIMATOR_3_OUTPUT = 2,
    eMIC_FILTER_DEBUG_SIG_DC_REMOVAL_OUTPUT = 3
};

int32_t mic_filter_get_debug_sig(uint32_t sig);

#endif
