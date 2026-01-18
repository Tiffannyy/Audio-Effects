/*
 * realtime_callback.h
 * DSP Program
 * 
 * Tiffany Liu
 * 5 June 2025
 * 
 * Description: Contains the callback function for realtime / continuous audio processing
 * 
*/


#ifndef REALTIME_CALLBACK_H
#define REALTIME_CALLBACK_H

#include <cmath>

// User Definitions
#define SAMPLE_SILENCE  0.0f

inline float toFloat(SAMPLE val){
	return val / 32768.0f;
}

inline SAMPLE toSample(float val){
	if (val > 1.0f) val = 1.0f;
	if (val < -1.0f) val = -1.0f;
	return (SAMPLE)(val * 32767.0f);
}


// Callback Function
static inline void processBlock(const SAMPLE* inputBuffer, SAMPLE* outputBuffer,
                     unsigned long framesPerBuffer,
                     RtUserData* ud){

    SAMPLE *out = outputBuffer;
    const SAMPLE *in = inputBuffer;

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        SAMPLE inL = *in++;
	SAMPLE inR = *in++;
	float inFloat = toFloat((inL + inR) / 2.0);
	float outFloat = inFloat;
        
        // No effect
        if (ud->effects->norm)
            outFloat = inFloat;

        // Tremolo effect
        else if (ud->effects->trem){
            double trem =    (1.0 - ud->params->TREM_DEPTH) + ud->params->TREM_DEPTH
                                * (0.5 * (1.0 + sin(ud->params->tremPhase)));
            
            ud->params->tremPhase += ud->tremIncrement;

            if (ud->params->tremPhase >= 2.0 * ud->params->PI) ud->params->tremPhase -= 2.0 * ud->params->PI;
        
            outFloat = inFloat * trem; 
        }
        

        // Delay effect
        else if (ud->effects->delay){
            float delayedSample = SAMPLE_SILENCE;

            if (!ud->delayBuffer.empty())
                delayedSample = ud->delayBuffer[ud->delayIndex];

            // store current input sample in delay buffer
            ud->delayBuffer[ud->delayIndex] = inFloat + delayedSample * ud->params->FEEDBACK;
            
            // Mix original and delayed signals
            outFloat = (1.0 - ud->params->MIX) * inFloat
                        + ud->params->MIX * delayedSample;
            
            // Increment and wrap delay index
            ud->delayIndex = (ud->delayIndex + 1) % ud->delaySize;
        }


        // Reverb
        else if (ud->effects->reverb){
            float outReverb = SAMPLE_SILENCE;

            for(int tap = 0; tap < AudioParams::REVERB_TAPS; ++tap){
                int i = (ud->reverbIndex[tap] + ud->reverbSize - ud->reverbDelay[tap]) % ud->reverbSize;
                float delayedSample = ud->reverbBuffer[i];

                outReverb += delayedSample * ud->reverbGain[tap];

                // update buffer with input + feedback
                ud->reverbBuffer[ud->reverbIndex[tap]] = inFloat + delayedSample * ud->params->reverbDecay;

                ud->reverbIndex[tap] = (ud->reverbIndex[tap] + 1) % ud->reverbSize;
            }

            outFloat = (1.0f - ud->params->MIX) * inFloat + ud->params->MIX * outReverb;
        }

        // Bitcrush
        else if (ud->effects->bitcrush) {
            // inputSample
            float outBitcrush = SAMPLE_SILENCE;

            // Calculate number of samples to hold
            double sampleCount = ud->params->SAMPLE_RATE / ud->params->DOWNSAMPLE_RATE;

            // Perform downsampling
            if (ud->bitcrushCount >= sampleCount) {
                // If bitcrush counter exceeds sample count, decrement counter & store new sample
                ud->bitcrushCount -= sampleCount;
                ud->bitcrushSample = inFloat;
                outBitcrush = inFloat;
            } else {
                // Else, increment counter and reuse stored sample
                ud->bitcrushCount++;
                outBitcrush = ud->bitcrushSample;
            }

            // Perform quantization
            double amplitudeStep = 1.0 / pow(2, ud->params->BIT_DEPTH);
            int quantizedValue = outBitcrush / amplitudeStep;
            outBitcrush = (double) quantizedValue / pow(2, ud->params->BIT_DEPTH);

            // Apply mix amount
            outFloat = (1.0f - ud->params->MIX) * inFloat + ud->params->MIX * outBitcrush;
        }

        else
            outFloat = inFloat;

	SAMPLE outSample = toSample(outFloat);

	*out++ = outSample;
	*out++ = outSample;
    }
}
#endif //REALTIME_CALLBACK_H
