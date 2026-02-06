/*
 * callback.cpp
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 5 February 2026
 * 
 * Description: Implementation of callback function.
 * Contains processing logic.
*/

#include "../include/callback.h"
#include <cmath>

// Callback Function
static inline void processBlock(const SAMPLE* in, SAMPLE* out,
                     unsigned long framesPerBuffer,
                     RtUserData* ud){

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        SAMPLE inL = *in++;
        SAMPLE inR = *in++;
        float inFloatL = toFloat(inL);
        float inFloatR = toFloat(inR);

    	float outL = inFloatL;
    	float outR = inFloatR;

        // No effect
        if (ud->effects->norm)
            outL = inFloatL;

        // Tremolo effect
        else if (ud->effects->trem){
            int j = (int)(ud->params->tremPhase * (ud->LUT_SIZE / (2.0f * M_PI))) & (ud->LUT_SIZE - 1);
            float trem =    (1.0 - ud->params->TREM_DEPTH) + ud->params->TREM_DEPTH
                                * (0.5 * (1.0 + ud->sineLUT[j]));
            
            ud->params->tremPhase += ud->tremIncrement;

            if (ud->params->tremPhase >= 2.0 * M_PI) ud->params->tremPhase -= 2.0 * M_PI;
        
            outL = inFloatL * trem; 
        }
        

        // Delay effect
        else if (ud->effects->delay){
            float delayedSample = SAMPLE_SILENCE;

            delayedSample = ud->delayBuffer[ud->delayIndex];

            // store current input sample in delay buffer
            ud->delayBuffer[ud->delayIndex] = inFloatL + delayedSample * ud->params->FEEDBACK;
            
            // Mix original and delayed signals
            outL = (1.0 - ud->params->MIX) * inFloatL
                        + ud->params->MIX * delayedSample;
            
            // Increment and wrap delay index
            ud->delayIndex++;
            if (ud->delayIndex >= ud->delaySize)
                ud->delayIndex = 0;
        }


        // Reverb
        else if (ud->effects->reverb){
            float outReverb = SAMPLE_SILENCE;
	        float feedbackSum = SAMPLE_SILENCE;

            for (int tap = 0; tap < AudioParams::REVERB_TAPS; tap++){
                int j = (ud->reverbIndex[tap] + ud->reverbSize - ud->reverbDelay[tap]) % ud->reverbSize;
                float delayedSample = ud->reverbBuffer[j];
                outReverb += delayedSample * ud->reverbGain[tap];

                // update buffer with input + feedback
                ud->reverbBuffer[ud->reverbIndex[tap]] = inFloatL + feedbackSum * ud->params->reverbDecay;
	        }

            ud->reverbBuffer[ud->reverbIndex[0]] = inFloatL + feedbackSum * ud->params->reverbDecay;

            for (int tap = 0; tap < AudioParams::REVERB_TAPS; tap++){
            ud->reverbIndex[tap]++;
            if (ud->reverbIndex[tap] >= ud->reverbSize)
                ud->reverbIndex[tap] = 0;
            }

            outL = (1.0f - ud->params->MIX) * inFloatL + ud->params->MIX * outReverb;
            }

        // Bitcrush
        else if (ud->effects->bitcrush) {
            // Calculate number of samples to hold
            float sampleCount = ud->params->SAMPLE_RATE / ud->params->DOWNSAMPLE_RATE;

            // Perform downsampling
            if (ud->bitcrushCount >= sampleCount) {
                // If bitcrush counter exceeds sample count, decrement counter & store new sample
                ud->bitcrushCount -= sampleCount;
                ud->bitcrushSample = inFloatL;
		        outL = inFloatL;
            }
            else{
                ud->bitcrushCount++;
                outL = ud->bitcrushSample;
            }
	
            float outBitcrush = ud->bitcrushSample;
            float step = ud->params->BITCRUSH_STEP;

            // Perform quantization
	        outBitcrush = roundf(outBitcrush / step) * step;
	        // Apply mix amount
            outL = (1.0f - ud->params->MIX) * inFloatL + ud->params->MIX * outBitcrush;
        }

        else{
            outL = inFloatL;
	        outR = inFloatR;
	    }

        *out++ = toSample(outL);
        *out++ = toSample(outR);
    }
}