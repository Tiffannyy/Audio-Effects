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
using namespace std;

// User Definitions
#define SAMPLE_SILENCE  0.0f


// Callback Function
static int streamCallback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeinfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData){
                
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    RtUserData *ud = (RtUserData*) userData;
    
    (void) timeinfo; // Prevent unused variable warnings.
    (void) statusFlags;


    if (inputBuffer == NULL){
        for(unsigned long i = 0; i < framesPerBuffer; i++){
            *out++ = SAMPLE_SILENCE;  // left
            *out++ = SAMPLE_SILENCE;  // right
        }
        return paContinue;
    }

    
    for(unsigned long i = 0; i < framesPerBuffer; i++){

        SAMPLE inputSample = *in++; // mono input
        
        // No effect
        if (ud->effects->norm){
            *out++ = inputSample;  // left
            *out++ = inputSample;  // right
        }
        

        // Tremolo effect
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
        

        // Delay effect
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


        // Reverb
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


        // Bitcrush
        else if (ud->effects->bitcrush) {
            // inputSample
            SAMPLE outputSample = SAMPLE_SILENCE;

            // Calculate number of samples to hold
            double sampleCount = ud->params->SAMPLE_RATE / ud->params->DOWNSAMPLE_RATE;

            // Perform downsampling
            if (ud->bitcrushCount >= sampleCount) {
                // If bitcrush counter exceeds sample count, decrement counter & store new sample
                ud->bitcrushCount -= sampleCount;
                ud->bitcrushSample = inputSample;
                outputSample = inputSample;
            } else {
                // Else, increment counter and reuse stored sample
                ud->bitcrushCount++;
                outputSample = ud->bitcrushSample;
            }

            // Perform quantization
            double amplitudeStep = 1.0 / pow(2, ud->params->BIT_DEPTH);
            int quantizedValue = outputSample / amplitudeStep;
            outputSample = (double) quantizedValue / pow(2, ud->params->BIT_DEPTH);

            // Apply mix amount
            outputSample = (1.0f - ud->params->MIX) * inputSample + ud->params->MIX * outputSample;

            *out++ = outputSample;
            *out++ = outputSample;
        }


    }
    return paContinue;
}
#endif //REALTIME_CALLBACK_H
