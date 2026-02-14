
#include <stdlib.h>
#include <string.h>

#include "cic.h"

#define ARRAY_SIZE(_array)              (sizeof(_array))
#define ARRAY_LEN(_array)               (sizeof(_array)/sizeof(_array[0]))
#define ARRAY_LAST_ELEMENT(_array)      (_array[ARRAY_LEN(_array) - 1])

int32_t cic_init(struct cic* const handler, uint8_t log2_decimation, int8_t output_gain)
{
    memset(handler->integrator_state, 0x00, ARRAY_SIZE(handler->integrator_state));
    memset(handler->comb_state, 0x00, ARRAY_SIZE(handler->comb_state));
    handler->last_output = 0;
    handler->input_count = 0;
    handler->input_count_mask = ((0x1 << log2_decimation) - 1);
    handler->output_gain = output_gain;
    return 0;
}

int32_t cic_push(struct cic* const handler, uint32_t sample)
{
    uint32_t integrator_input = sample;
    for (size_t i = 0; i < ARRAY_LEN(handler->integrator_state); i++)
    {
        handler->integrator_state[i] += integrator_input;
        integrator_input = handler->integrator_state[i];
    }

    handler->input_count = ((handler->input_count + 1) & handler->input_count_mask);
    if (0 != handler->input_count)
    {
        return 0;
    }

    uint32_t comb_outputs[ARRAY_LEN(handler->comb_state)] = {0};
    uint32_t comb_input = ARRAY_LAST_ELEMENT(handler->integrator_state);
    for (size_t i = 0; i < ARRAY_LEN(comb_outputs); i++)
    {
        comb_outputs[i] = comb_input - handler->comb_state[i];
        comb_input = comb_outputs[i];
    }
    comb_input = ARRAY_LAST_ELEMENT(handler->integrator_state);
    for (size_t i = 0; i < ARRAY_LEN(comb_outputs); i++)
    {
        handler->comb_state[i] = comb_input;
        comb_input = comb_outputs[i];
    }

    uint32_t comb_section_output = ARRAY_LAST_ELEMENT(comb_outputs);
    if (0 <= handler->output_gain)
    {
        handler->last_output = (int16_t)((int32_t)(comb_section_output << handler->output_gain) - (int32_t)(32768));
    }
    else
    {
        handler->last_output = (int16_t)((int32_t)(comb_section_output >> abs(handler->output_gain)) - (int32_t)(32768));
    }
    
    return 1;
}

int16_t cic_get_last_output(struct cic* const handler)
{
    return handler->last_output;
}


struct cic* test_alloc_struct_cic(void)
{
    return (struct cic*)malloc(sizeof(struct cic));
}

void test_free_struct_cic(struct cic* pmem)
{
    free((void*)pmem);
}
