#ifndef _AUDIO_TIMER_H_
#define _AUDIO_TIMER_H_

#include <stdint.h>

int audio_timer_get(uint32_t* ticks);
int audio_timer_reset(void);

#endif