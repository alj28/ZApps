#ifndef _CIC_H_
#define _CIC_H_

#include <stdint.h>

#define MIC_FILTER_CIC_ORDER    5

struct cic
{
    uint32_t integrator_state[MIC_FILTER_CIC_ORDER];
    uint32_t comb_state[MIC_FILTER_CIC_ORDER];
    int16_t last_output;
    uint8_t input_count;
    uint8_t input_count_mask;
    int8_t output_gain;
};

int32_t cic_init(struct cic* const handler, uint8_t log2_decimation, int8_t output_gain);
int32_t cic_push(struct cic* const handler, uint32_t sample);
int16_t cic_get_last_output(struct cic* const handler);

// for testgs
struct cic* test_alloc_struct_cic(void);
void test_free_struct_cic(struct cic* pmem);

#endif
