/*
 * callback.cpp
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 24 February 2026
 * 
 * Description: Implementation of callback function.
 * Contains processing logic.
*/

#include "../include/callback.h"
#include <cstdio>

// Overdrive function
float applyOverdrive(float inputSample,
		     float drive,
		     float odFactor) {

    // Apply transfer characteristic
    float intensityFactor = 1.0 / (odFactor*drive + 0.01);
    float normalizeFactor = 1.0 / (intensityFactor + 1.0);
    float outputSample = (inputSample / (intensityFactor + std::fabs(inputSample)));
    outputSample /= normalizeFactor;

    return outputSample;
}

// Distortion function
float applyDistortion(float inputSample,
		      float drive,
		      float distFactor) {
    
    // Apply transfer characteristic
    float outputSample = (1 + (distFactor-1)*drive) * inputSample;
    if (outputSample > 1.0f) outputSample = 1.0f;
    if (outputSample < -1.0f) outputSample = -1.0f;

    return outputSample;
}

// Fuzz function
float applyFuzz(float inputSample,
		float drive,
		float fuzzFactor,
		RtUserData* ud) {

    // Adjust average amplitude for reactive biasing
    ud->fuzzSampleAvg = ud->fuzzSampleAvg + (fmin(1.414*abs(inputSample), 1.0) - ud->fuzzSampleAvg) / ud->params->FUZZ_ATTACK;
    ud->params->FUZZ_MAX_BIAS *= ud->fuzzSampleAvg;

    // Apply transfer characteristic
    float intensityFactor = 1 / (fuzzFactor*drive + 0.01);
    float biasFactor      = ud->params->FUZZ_MAX_BIAS * drive;
    float normalizeFactor;
    if (inputSample >= -biasFactor)
        normalizeFactor = (1 + biasFactor) / (intensityFactor + abs(1 + biasFactor));
    else
        normalizeFactor = (1 - biasFactor) / (intensityFactor + abs(-1 + biasFactor));
    float outputSample = (inputSample + biasFactor) / (intensityFactor + abs(inputSample + biasFactor));
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
void processBlock(const SAMPLE* in, SAMPLE* out,
                     unsigned long framesPerBuffer,
                     RtUserData* ud,
		     const ParamSnapshot& params){

    const float tremIncrement = 2.0f * M_PI * params.tremFreq / params.sampleRate;

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        float inL = *in++;
        float inR = *in++;
        float inFloatL = toFloat(inL);
	float inFloatR = toFloat(inR);

	float monoIn = 0.5f * (inFloatL + inFloatR);
	float monoOut = monoIn;

        // No effect
        if (ud->effects->norm.load())
            monoOut = monoIn;

        // Tremolo effect
        else if (ud->effects->trem.load()){
            int j = (int)(ud->params->TREM_PHASE * (ud->LUT_SIZE / (2.0f * M_PI))) & (ud->LUT_SIZE - 1);
            float trem = (1.0 - params.tremDepth) + params.tremDepth * (0.5 * (1.0 + ud->sineLUT[j]));
            
             ud->params->TREM_PHASE += tremIncrement;

            if (ud->params->TREM_PHASE >= 2.0 * M_PI) ud->params->TREM_PHASE -= 2.0 * M_PI;
        
            //monoOut = monoIn * trem; 
            monoOut = (1.0f - params.mix) * monoIn + params.mix * monoIn * trem;

        }
        

        // Delay effect
        else if (ud->effects->delay.load()){
            float delayedSample = SAMPLE_SILENCE;
            // add
            delayedSample = ud->delayBuffer[ud->delayIndex];
        
            // store current input sample in delay buffer
            ud->delayBuffer[ud->delayIndex] = monoIn + delayedSample * params.delayFeedback;
            
            // Mix original and delayed signals
            monoOut = (1.0 - params.mix) * monoIn
                        + params.mix * delayedSample;
            
            // Increment and wrap delay index
            ud->delayIndex++;
            if (ud->delayIndex >= ud->delaySize)
                ud->delayIndex = 0;
        }


        // Reverb
        else if (ud->effects->reverb.load()){
            float outReverb = SAMPLE_SILENCE;
	        float feedbackSum = SAMPLE_SILENCE;

            for (int tap = 0; tap < AudioParams::REVERB_TAPS; tap++){
                int j = (ud->reverbIndex[tap] + ud->reverbSize - ud->reverbDelay[tap]) % ud->reverbSize;
                float delayedSample = ud->reverbBuffer[j];
                outReverb += delayedSample * ud->reverbGain[tap];

                // update buffer with input + feedback
                ud->reverbBuffer[ud->reverbIndex[tap]] = monoIn + feedbackSum * params.reverbDecay;
	        }

            ud->reverbBuffer[ud->reverbIndex[0]] = monoIn + feedbackSum * params.reverbDecay;

            for (int tap = 0; tap < AudioParams::REVERB_TAPS; tap++){
                ud->reverbIndex[tap]++;
                if (ud->reverbIndex[tap] >= ud->reverbSize)
                    ud->reverbIndex[tap] = 0;
            }

            monoOut = (1.0f - params.mix) * monoIn + params.mix * outReverb;
            }

        // Bitcrush
        else if (ud->effects->bitcrush.load()) {
            // Calculate number of samples to hold
            float sampleCount = params.sampleRate / params.bitcrushRate;

            // Perform downsampling
            if (ud->bitcrushCount >= sampleCount) {
                // If bitcrush counter exceeds sample count, decrement counter & store new sample
                ud->bitcrushCount -= sampleCount;
                ud->bitcrushSample = monoIn;
		        monoOut = monoIn;
            }
            else{
                ud->bitcrushCount++;
                monoOut = ud->bitcrushSample;
            }
	
            float outBitcrush = ud->bitcrushSample;
            float step = 2.0f / (1 << params.bitcrushDepth); //ud->params->BITCRUSH_STEP;

            // Perform quantization
	        outBitcrush = roundf(outBitcrush / step) * step;
	        // Apply mix amount
            monoOut = (1.0f - params.mix) * monoIn + params.mix * outBitcrush;
        }

        // Overdrive
        else if (ud->effects->overdrive.load()) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyOverdrive(monoIn, params.odDrive, ud->params->OD_FACTOR);
            float filteredSample = applyToneFilter(distortedSample, ud,
                                                ud->odToneBuffer,
                                                params.odTone);
            outputSample = filteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < 1.0f) outputSample = -1.0f;

            // Apply mix amount
            monoOut = (1.0f - params.mix) * monoIn + params.mix * outputSample;
            
        }

        // Distortion
        else if (ud->effects->distortion.load()) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyDistortion(monoIn, params.distDrive, ud->params->DIST_FACTOR);

            float filteredSample = applyToneFilter(distortedSample, ud, ud->distToneBuffer, params.distTone);
            outputSample = filteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < -1.0f) outputSample = -1.0f;

            // Apply mix amount
            monoOut = (1.0f - params.mix) * monoIn + params.mix * outputSample;
        }

        // Fuzz
        else if (ud->effects->fuzz.load()) {
            float outputSample = SAMPLE_SILENCE;

            // Apply effect and filters
            float distortedSample = applyFuzz(monoIn, params.fuzzDrive, ud->params->FUZZ_FACTOR, ud);
            float filteredSample = applyToneFilter(distortedSample, ud, ud->fuzzToneBuffer, params.fuzzTone);
            float dcFilteredSample = applyDCFilter(filteredSample, ud);
            outputSample = dcFilteredSample;

            // Adjust for overflow
            if (outputSample > 1.0f) outputSample = 1.0f;
            else if (outputSample < -1.0f) outputSample = -1.0f;

            // Apply mix amount
            monoOut = (1.0f - params.mix) * monoIn + params.mix * outputSample;
        }

        else
            monoOut = monoIn;

        *out++ = ud->params->VOLUME.load() * toSample(monoOut);
        *out++ = ud->params->VOLUME.load() * toSample(monoOut);
    }
}

