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


// Overdrive function
SAMPLE applyOverdrive(SAMPLE inputSample, RtUserData *ud) {
    
    // Effect parameters
    float drive    = ud->params->OD_DRIVE;
    float odFactor = ud->params->OD_FACTOR;

    // Apply transfer characteristic
    float intensityFactor = 1 / (odFactor*drive + 0.01);
    float normalizeFactor = 1 / (intensityFactor + 1);
    SAMPLE outputSample = (inputSample / (intensityFactor + abs(inputSample)));
    outputSample /= normalizeFactor;

    return outputSample;
}


// Distortion function
SAMPLE applyDistortion(SAMPLE inputSample, RtUserData *ud) {
    
    // Effect parameters
    float drive      = ud->params->DIST_DRIVE;
    float distFactor = ud->params->DIST_FACTOR;

    // Apply transfer characteristic
    SAMPLE outputSample = (1 + (distFactor-1)*drive) * inputSample;
    if (outputSample > 1.0f) outputSample = 1.0f;
    if (outputSample < -1.0f) outputSample = -1.0f;

    return outputSample;
}


// Fuzz function
SAMPLE applyFuzz(SAMPLE inputSample, RtUserData *ud) {
    
    // Effect parameters
    float drive       = ud->params->FUZZ_DRIVE;
    float fuzzFactor  = ud->params->FUZZ_FACTOR;
    float fuzzBias    = ud->params->FUZZ_MAX_BIAS;
    int   fuzzAttack  = ud->fuzzSampleCount;

    // Adjust average amplitude for reactive biasing
    ud->fuzzSampleAvg = ud->fuzzSampleAvg + (min(1.414*abs(inputSample), 1.0) - ud->fuzzSampleAvg) / fuzzAttack;
    fuzzBias *= ud->fuzzSampleAvg;

    // Apply transfer characteristic
    float intensityFactor = 1 / (fuzzFactor*drive + 0.01);
    float biasFactor      = fuzzBias * drive;
    float normalizeFactor;
    if (inputSample >= -biasFactor)
        normalizeFactor = (1 + biasFactor) / (intensityFactor + abs(1 + biasFactor));
    else
        normalizeFactor = (1 - biasFactor) / (intensityFactor + abs(-1 + biasFactor));
    SAMPLE outputSample = (inputSample + biasFactor) / (intensityFactor + abs(inputSample + biasFactor));
    outputSample /= normalizeFactor;

    return outputSample;
}


// Tone filter function
SAMPLE applyToneFilter(SAMPLE inputSample, RtUserData *ud, SAMPLE* filterBuffer, float toneAmount) {

    /* NOTE: This was generalized for all 3 distortion effects, so buffer array
     * and tone parameter are directly passed to reduce calculation. */

    // Shift filter buffers over
    for (int i = ud->params->TONE_SIZE - 2; i >= 0; i--)
        filterBuffer[i+1] = filterBuffer[i];
    filterBuffer[0] = inputSample;

    // Apply tone coefficients for lowpass filter
    SAMPLE outputSample = SAMPLE_SILENCE;
    for (int i = 0; i < ud->params->TONE_SIZE; i++)
        outputSample += ud->params->TONE_COEFFICIENTS[i] * filterBuffer[i];

    // Apply mix amount
    outputSample = toneAmount * inputSample + (1.0f - toneAmount) * outputSample;

    return outputSample;
}


// DC filter function
SAMPLE applyDCFilter(SAMPLE inputSample, RtUserData *ud) {

    // Apply IIR equation for DC filter
    // y[n] = x[n] - x[n-1] + Ry[n-1]
    SAMPLE outputSample = inputSample - ud->dcInputBuffer + (ud->params->DC_POLE_COEFFICENT)*ud->dcOutputBuffer;
    ud->dcInputBuffer = inputSample;
    ud->dcOutputBuffer = outputSample;

    // Apply mix amount
    outputSample = ud->params->DC_MIX * outputSample + (1.0f - ud->params->DC_MIX) * inputSample;

    return outputSample;
}


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


        // Overdrive 
        else if (ud->effects->overdrive) {
            SAMPLE outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            SAMPLE distortedSample = applyOverdrive(inputSample, ud);
            SAMPLE filteredSample = applyToneFilter(distortedSample, ud, ud->odToneBuffer, ud->params->OD_TONE);
            outputSample = filteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            // Apply mix amount
            outputSample = (1.0f - ud->params->MIX) * inputSample + ud->params->MIX * outputSample;

            *out++ = outputSample;
            *out++ = outputSample;
        }


        // Distortion
        else if (ud->effects->distortion) {
            SAMPLE outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            SAMPLE distortedSample = applyDistortion(inputSample, ud);
            SAMPLE filteredSample = applyToneFilter(distortedSample, ud, ud->distToneBuffer, ud->params->DIST_TONE);
            outputSample = filteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            // Apply mix amount
            outputSample = (1.0f - ud->params->MIX) * inputSample + ud->params->MIX * outputSample;

            *out++ = outputSample;
            *out++ = outputSample;
        }


        // Fuzz
        else if (ud->effects->fuzz) {
            SAMPLE outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            SAMPLE distortedSample = applyFuzz(inputSample, ud);
            SAMPLE filteredSample = applyToneFilter(distortedSample, ud, ud->fuzzToneBuffer, ud->params->FUZZ_TONE);
            SAMPLE dcFilteredSample = applyDCFilter(filteredSample, ud);
            outputSample = dcFilteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            *out++ = outputSample;
            *out++ = outputSample;
        }


    }

    return paContinue;
}



#endif //REALTIME_CALLBACK_H
