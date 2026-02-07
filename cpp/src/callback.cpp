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
    ud->fuzzSampleAvg = ud->fuzzSampleAvg + (fmin(1.414*abs(inputSample), 1.0) - ud->fuzzSampleAvg) / fuzzAttack;
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
float applyToneFilter(float inputSample, RtUserData *ud, float* filterBuffer, float toneAmount) {

    /* NOTE: This was generalized for all 3 distortion effects, so buffer array
     * and tone parameter are directly passed to reduce calculation. */

    // Shift filter buffers over
    for (int i = ud->params->TONE_SIZE - 2; i >= 0; i--)
        filterBuffer[i+1] = filterBuffer[i];
    filterBuffer[0] = inputSample;

    // Apply tone coefficients for lowpass filter
    float outputSample = SAMPLE_SILENCE;
    for (int i = 0; i < ud->params->TONE_SIZE; i++)
        outputSample += ud->params->TONE_COEFFICIENTS[i] * filterBuffer[i];

    // Apply mix amount
    outputSample = toneAmount * inputSample + (1.0f - toneAmount) * outputSample;

    return outputSample;
}


// DC filter function
float applyDCFilter(float inputSample, RtUserData *ud) {

    // Apply IIR equation for DC filter
    // y[n] = x[n] - x[n-1] + Ry[n-1]
    float outputSample = inputSample - ud->dcInputBuffer + (ud->params->DC_POLE_COEFFICENT)*ud->dcOutputBuffer;
    ud->dcInputBuffer = inputSample;
    ud->dcOutputBuffer = outputSample;

    // Apply mix amount
    outputSample = ud->params->DC_MIX * outputSample + (1.0f - ud->params->DC_MIX) * inputSample;

    return outputSample;
}

// Callback Function
static inline void processBlock(const float* in, float* out,
                     unsigned long framesPerBuffer,
                     RtUserData* ud){

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        float inL = *in++;
        float inR = *in++;
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

        // Overdrive
        else if (ud->effects->overdrive) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyOverdrive(inFloatL, ud);
            float filteredSample = applyToneFilter(distortedSample, ud,
                                                ud->odToneBuffer,
                                                ud->params->OD_TONE);

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < 1.0f) outputSample = -1.0f;

            // Apply mix amount
            outL = (1.0f - ud->params->MIX) * inFloatL + ud->params->MIX * outputSample;
        }

        // Distortion
        else if (ud->effects->distortion) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyDistortion(inFloatL, ud);
            float filteredSample = applyToneFilter(distortedSample, ud, ud->distToneBuffer, ud->params->DIST_TONE);
            outputSample = filteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < -1.0f) outputSample = -1.0f;

            // Apply mix amount
            outL = (1.0f - ud->params->MIX) * inFloatL + ud->params->MIX * outputSample;
        }

        // Fuzz
        else if (ud->effects->fuzz) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyFuzz(inFloatL, ud);
            float filteredSample = applyToneFilter(distortedSample, ud, ud->fuzzToneBuffer, ud->params->FUZZ_TONE);
            float dcFilteredSample = applyDCFilter(filteredSample, ud);
            outputSample = dcFilteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < -1.0f) outputSample = -1.0f;

            outL = outputSample;
        }

        else{
            outL = inFloatL;
	        outR = inFloatR;
	    }

        *out++ = toSample(outL);
        *out++ = toSample(outR);
    }
}