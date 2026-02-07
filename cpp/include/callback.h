/*
 * callback.h
 * 
 * Tiffany Liu
 * 5 June 2025
 * 
 * Description: Declaration of callback
 * 
*/

#pragma once

#include "types.h"
#define SAMPLE_SILENCE 0.0f

float applyToneFilter(SAMPLE inputSample, RtUserData *ud, SAMPLE* filterBuffer, float toneAmount);


inline float toFloat(SAMPLE val){
	return val / 32768.0f;
}

inline SAMPLE toSample(float val){
	if (val > 1.0f) val = 1.0f;
	if (val < -1.0f) val = -1.0f;
	return (SAMPLE)(val * 32767.0f);
}

static inline void processBlock(const SAMPLE* in, SAMPLE* out,
                unsigned long framesPerBuffer,
                RtUserData* ud);
