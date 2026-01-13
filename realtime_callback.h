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


// Callback Function
static inline void processBlock(const SAMPLE* inputBuffer, SAMPLE* outputBuffer,
                     unsigned long framesPerBuffer,
                     RtUserData* ud){

    SAMPLE *out = outputBuffer;
    const SAMPLE *in = inputBuffer;

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        SAMPLE left = *in++;
        SAMPLE right = *in++;
        SAMPLE inputSample = 0.5f * (left + right);
        
        // no effect
        if (ud->effects->norm){
            *out++ = 0.5f * 440;//inputSample;  // left
            *out++ = inputSample;  // right
        }
        
        // tremolo effect
        else if (ud->effects->trem){
            double tremolo =    (1.0 - ud->params->TREM_DEPTH) + ud->params->TREM_DEPTH
                                * (0.5 * (1.0 + sin(ud->params->tremPhase)));
            
            ud->params->tremPhase += ud->tremIncrement;

            if (ud->params->tremPhase >= 2.0 * ud->params->PI) ud->params->tremPhase -= 2.0 * ud->params->PI;
            
            if (inputSample > 1.0f)  inputSample = 1.0f;
            if (inputSample < -1.0f) inputSample = -1.0f;

            *out++ = inputSample * tremolo;  // left
            *out++ = inputSample * tremolo;  // right
        }
        
        // delay effect
        else if (ud->effects->delay){
            SAMPLE delayedSample = SAMPLE_SILENCE;

            if (!ud->delayBuffer.empty())
                delayedSample = ud->delayBuffer[ud->delayIndex];

            // store current input sample in delay buffer
            ud->delayBuffer[ud->delayIndex] = inputSample + delayedSample * ud->params->FEEDBACK;
            
            // Mix original and delayed signals
            SAMPLE outputSample =   (1.0 - ud->params->MIX) * inputSample
                                    + ud->params->MIX * delayedSample;
            
            // Increment and wrap delay index
            ud->delayIndex = (ud->delayIndex + 1) % ud->delaySize;
            
            if (outputSample > 1.0f)  outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            *out++ = outputSample;  // left
            *out++ = outputSample;  // right
        }

        // reverb
        else if (ud->effects->reverb){
            SAMPLE outputSample = SAMPLE_SILENCE;

            for(int tap = 0; tap < AudioParams::REVERB_TAPS; ++tap){
                int i = (ud->reverbIndex[tap] + ud->reverbSize - ud->reverbDelay[tap]) % ud->reverbSize;
                SAMPLE delayedSample = ud->reverbBuffer[i];

                outputSample += delayedSample * ud->reverbGain[tap];

                // update buffer with input + feedback
                ud->reverbBuffer[ud->reverbIndex[tap]] = inputSample + delayedSample * ud->params->reverbDecay;

                ud->reverbIndex[tap] = (ud->reverbIndex[tap] + 1) % ud->reverbSize;
            }

            outputSample = (1.0f - ud->params->MIX) * inputSample + ud->params->MIX * outputSample;

            if (outputSample > 1.0f) outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            *out++ = outputSample;
            *out++ = outputSample;
        }

        else {
            *out++ = inputSample;
            *out++ = inputSample;
        }
    }
}
#endif //REALTIME_CALLBACK_H
