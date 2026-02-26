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
#include <cmath>

#define SAMPLE_SILENCE 0.0f

struct ParamSnapshot {
    float mix;
    float sampleRate;

    float tremFreq;
    float tremDepth;

    int delayMs;
    float delayFeedback;

    float reverbDecay;

    int bitcrushRate;
    int bitcrushDepth;

    float odDrive;
    float odTone;
	    
    float distDrive;
    float distTone;

    float fuzzDrive;
    float fuzzTone;

};

float applyToneFilter(SAMPLE inputSample,
		     RtUserData *ud,
		     SAMPLE* filterBuffer,
		     float toneAmount);

inline float toFloat(SAMPLE val){
	return val / 32768.0f;
}

inline SAMPLE toSample(float val){
	if (val > 1.0f) val = 1.0f;
	if (val < -1.0f) val = -1.0f;
	return (SAMPLE)(val * 32767.0f);
}

void processBlock(
	const SAMPLE* in,
    	SAMPLE* out,
    	unsigned long framesPerBuffer,
    	RtUserData* ud,
    	const ParamSnapshot& params);
