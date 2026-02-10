/*
 * audio.h
 * 
 * Tiffany Liu
 * 9 February 2026
 * 
 * Contains initialization, reset, and stream functions for engine
*/

#include "callback.h"
#include "types.h"

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <unistd.h>
#include <atomic>
#include <vector.h>

int setupPCM(const char* device, snd_pcm_t** handle,
            snd_pcm_stream_t stream, unsigned int channels,
            unsigned int rate, snd_pcm_uframes_t period,
            snd_pcm_uframes_t buffer);

void initData(RtUserData &ud, AudioParams &audioParams,
              EffectChoices &effectChoice);

void resetData(RtUserData &ud);

void stream(RtUserData &userData, AudioParams &audioParams,
            EffectChoices &effectChoice,
            snd_pcm_t *inHandle, snd_pcm_t *outHandle,
	        snd_pcm_uframes_t period,
            std::atomic<bool> &running);
