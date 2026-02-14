
#include <string.h>

#include "decimator.h"


int32_t decimator_init(struct decimator* const handler, q15_t* taps, q15_t* states, 
                        size_t len, q15_t* inputs, uint8_t decimation)
{
    handler->inputs = inputs;
    handler->last_output = 0;
    handler->decimation = decimation;
    handler->input_count = 0;

    arm_status rv = arm_fir_decimate_init_q15(
        &(handler->cmsis_filter),
        len,
        decimation,
        taps,
        states,
        decimation
    );

    return (ARM_MATH_SUCCESS == rv) ? (0) : (1);
}

int32_t decimator_push(struct decimator* const handler, q15_t sample)
{
    handler->inputs[handler->input_count] = sample;
    handler->input_count++;

    if (handler->decimation > handler->input_count)
    {
        return 0;
    }

    arm_fir_decimate_q15(
        &(handler->cmsis_filter),
        (handler->inputs),
        &(handler->last_output),
        handler->decimation
    );
    handler->input_count = 0;
    return 1;
}

q15_t decimator_get_last_output(struct decimator* const handler)
{
    return handler->last_output;
}
