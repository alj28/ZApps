
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "audio_stream.h"

LOG_MODULE_REGISTER(audio_stream, LOG_LEVEL_INF);


K_MEM_SLAB_DEFINE_STATIC(
    audio_stream_slab,
    sizeof(struct audio_chunk),
    5,
    4
);

K_QUEUE_DEFINE(
    audio_stream_queue
);

K_FIFO_DEFINE(
    audio_stream_fifo
);


struct audio_chunk* audio_stream_chunk_alloc(k_timeout_t timeout)
{
    struct audio_chunk* rv;
    int err = k_mem_slab_alloc(
        &audio_stream_slab,
        (void*)&rv,
        timeout
    );
    
    if (0 > err) {
        LOG_ERR("Cannot allocate audio stream chunk. (err %d)", err);
        rv = NULL;
    }
    
    return rv;
}

int audio_stream_chunk_commit(struct audio_chunk* chunk) 
{
    //k_queue_append(
    //    &audio_stream_queue,
    //    (void*)chunk
    //);
    k_fifo_put(
        &audio_stream_fifo,
        (void*)chunk
    );
    return 0;
}

int audio_stream_chunk_abort(struct audio_chunk* chunk)
{
    return audio_stream_chunk_release(chunk);
}

struct audio_chunk* audio_stream_chunk_get(k_timeout_t timeout)
{
    //return (struct audio_chunk*) k_queue_get(&audio_stream_queue, timeout);
    return (struct audio_chunk*) k_fifo_get(&audio_stream_fifo, timeout);
}

int audio_stream_chunk_release(struct audio_chunk *chunk)
{
    k_mem_slab_free(
        &audio_stream_slab,
        chunk
    );
    return 0;
}

