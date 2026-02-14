#ifndef _DECIMATOR_H_
#define _DECIMATOR_H_

#include <stdint.h>
#include "arm_math.h"

struct decimator
{
    q15_t* inputs;
    arm_fir_decimate_instance_q15 cmsis_filter;
    q15_t last_output;
    uint8_t decimation;
    uint8_t input_count;
};

int32_t decimator_init(struct decimator* const handler, q15_t* taps, q15_t* states, 
                        size_t len, q15_t* inputs, uint8_t decimation);
int32_t decimator_push(struct decimator* const handler, q15_t sample);
q15_t decimator_get_last_output(struct decimator* const handler);


#endif
