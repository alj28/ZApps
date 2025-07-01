
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "audio_stream.h"

LOG_MODULE_REGISTER(audio_stream, LOG_LEVEL_INF);

struct fifo_entry {
    void* fifo_reserved;
    struct audio_chunk data;
};

K_MEM_SLAB_DEFINE_STATIC(
    audio_stream_slab,
    sizeof(struct fifo_entry),
    5,
    4
);

K_QUEUE_DEFINE(
    audio_stream_queue
);

K_FIFO_DEFINE(
    audio_stream_fifo
);

static inline struct fifo_entry* audio_chunk_to_fifo_entry_ptr(struct audio_chunk* const audio_chunk) {
    return CONTAINER_OF(audio_chunk, struct fifo_entry, data);
}


struct audio_chunk* audio_stream_chunk_alloc(k_timeout_t timeout)
{
    struct fifo_entry* rv;
    int err = k_mem_slab_alloc(
        &audio_stream_slab,
        (void*)&rv,
        timeout
    );
    
    if (0 > err) {
        LOG_ERR("Cannot allocate audio stream chunk. (err %d)", err);
        rv = NULL;
    }
    
    return &rv->data;
}

int audio_stream_chunk_commit(struct audio_chunk* chunk) 
{
    struct fifo_entry* entry = audio_chunk_to_fifo_entry_ptr(chunk);
    k_fifo_put(
        &audio_stream_fifo,
        (void*)entry
    );
    return 0;
}

int audio_stream_chunk_abort(struct audio_chunk* chunk)
{
    return audio_stream_chunk_release(chunk);
}

struct audio_chunk* audio_stream_chunk_get(k_timeout_t timeout)
{
    return &((struct fifo_entry*) k_fifo_get(&audio_stream_fifo, timeout))->data;
}

int audio_stream_chunk_release(struct audio_chunk *chunk)
{
    struct fifo_entry* entry = audio_chunk_to_fifo_entry_ptr(chunk);
    k_mem_slab_free(
        &audio_stream_slab,
        (void*)entry
    );
    return 0;
}

