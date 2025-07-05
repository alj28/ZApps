#ifndef _AUDIO_STREAM_H_
#define _AUDIO_STREAM_H_

#include <stdint.h>

#include "audio_conf.h"

struct audio_chunk
{
    uint8_t mem[AUDIO_STREAM_CHUNK_SIZE];
    size_t size;
};

struct audio_chunk* audio_stream_chunk_alloc(k_timeout_t timeout);
int audio_stream_chunk_commit(struct audio_chunk* chunk);
int audio_stream_chunk_abort(struct audio_chunk* chunk);
struct audio_chunk* audio_stream_chunk_get(k_timeout_t timeout);
int audio_stream_chunk_release(struct audio_chunk *chunk);
size_t audio_stream_pending_chunk_count(void);

#endif 