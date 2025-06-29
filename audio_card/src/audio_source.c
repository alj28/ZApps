
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "audio_stream.h"
#include "audio_conf.h"
#include "test_signal_generator.h"

#define STACKSIZE 1024
#define PRIORITY 4

LOG_MODULE_REGISTER(audio_source, LOG_LEVEL_INF);




static void thread(void *p1, void *p2, void *p3)
{
    while (1) {
        struct audio_chunk* audio_chunk = audio_stream_chunk_alloc(K_FOREVER);
        generate_signal_section((int16_t*)audio_chunk->mem, SAMPLES_PER_SOF);
        audio_chunk->size = BYTES_PER_SOF;
        audio_stream_chunk_commit(audio_chunk);
        //LOG_INF("New chunk created.");
        //k_msleep(1000);
    }
}

K_THREAD_DEFINE(audio_source_thread_id, STACKSIZE, thread, NULL, NULL, NULL,
		PRIORITY, 0, 0);

