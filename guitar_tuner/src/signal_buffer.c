
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(signal_buffer, LOG_LEVEL_INF);

#define AUDIO_SAMPLES_SIZE      16384
#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEP_TIME_MS 10

static int32_t audio_samples[AUDIO_SAMPLES_SIZE]; 
static size_t audio_samples_indx = 0;

void push_to_signal_buffer(int32_t sample)
{
    audio_samples[audio_samples_indx] = sample;
    audio_samples_indx = (audio_samples_indx + 1) & ((1 << 14) - 1);
}

static void thread(void *p1, void *p2, void *p3)
{
    
    for(;;)
    {
        audio_samples[audio_samples_indx % AUDIO_SAMPLES_SIZE] = (audio_samples_indx);
        audio_samples_indx++;
        k_msleep(SLEEP_TIME_MS);
    }
}

//K_THREAD_DEFINE(signal_buffer_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
//		PRIORITY, 0, 0);
