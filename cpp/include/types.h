/*
 * types.h
 * DSP Program
 * 
 * Tiffany Liu
 * 5 June 2025
 * 
 * Description: contains structs used in the program.
 * NOTE: Change AudioParams values to adjust audio effects
 * 
*/

#ifndef TYPES_H
#define TYPES_H

#include <cmath>
#include <vector>

// User Defined Data
typedef int16_t SAMPLE;

// Primarily for menu and callback functions
struct EffectChoices{
    bool norm       = false;
    bool trem       = false;
    bool delay      = false;
    bool reverb     = false;
    bool bitcrush   = false;
    bool overdrive  = false;
    bool distortion = false;
    bool fuzz       = false;
};


// Parameters to pass to callback functions
struct AudioParams{
    static constexpr float PI = 3.14159265358979323846;

    float MIX           = 0.5;      // Mix between original and delayed signals

    // Tremolo
    float TREM_FREQ     = 4.0;      // tremolo frequency (Hz). lower the freq, the slower the tremolo effect vice versa
    float TREM_DEPTH    = 0.5;      // tremolo depth. 0 has no effect, 1 has full effect
    float tremPhase    = 0.1;

    // Delay
    static constexpr int DELAY_MS       = 500;      // delay in milliseconds
    static constexpr float FEEDBACK    = 0.4;     // feedback amount (0 to 1)   -  for delay

    // Reverb
    static const int REVERB_TAPS        = 5;        // number of delay taps for reverb
    static constexpr float reverbDecay  = 0.6;      // decay factor for reverb

    // Bitcrush
    int DOWNSAMPLE_RATE = 12000;     // Rate to "resample" input signal (Hz) (Must NOT exceed sample rate)
    static constexpr int BIT_DEPTH       = 8;        // Amount of bits to "quantize" sample amplitude
    static constexpr float BITCRUSH_STEP = 1.0f / (1 << BIT_DEPTH);

    // Overdrive
    float OD_DRIVE  = 1;
    float OD_TONE   = 1;
    float OD_FACTOR = 3;

    // Distortion
    float DIST_DRIVE  = 1;
    float DIST_TONE   = 1;
    float DIST_FACTOR = 3;

    // Fuzz
    float FUZZ_DRIVE    = 1;
    float FUZZ_TONE     = 1;
    float FUZZ_FACTOR   = 20;
    float FUZZ_MAX_BIAS = 0.6;   // Must be between -1 to 1
    static constexpr float FUZZ_ATTACK = 8;  // In milliseconds

    // Tone filter parameters
    // (The implementation of "tone" utilizes a windowed lowpass filter.)
    static const int TONE_SIZE = 10;
    float TONE_COEFFICIENTS[TONE_SIZE] = {
        0.0139,
        0.0353,
        0.0917,
        0.1594,
        0.2050,
        0.2050,
        0.1594,
        0.0917,
        0.0353,
        0.0139
    };

    // DC filter parameters
    // (An IIR with a zero at z = 1 and a pole "near" z = 1.)
    float DC_POLE_COEFFICENT = 0.995;
    float DC_MIX = 0.3;

    static constexpr int CHANNELS   = 2;
    static constexpr int SAMPLE_RATE   = 44100;
};


struct RtUserData {

    AudioParams *params;
    EffectChoices *effects;

    // Sin
    static constexpr int LUT_SIZE = 1024;      // look up table, less expensive than calling sin every iteration
    float sineLUT[LUT_SIZE];
    
    RtUserData(){
    	for (int i = 0; i < LUT_SIZE; i++)
    	    sineLUT[i] = sinf(2.0f * AudioParams::PI * i / LUT_SIZE);
    }

    // Delay
    std::vector<float> delayBuffer;
    int delayIndex;
    int delaySize;

    // Reverb
    std::vector<float> reverbBuffer;
    int   reverbSize;
    int   reverbIndex[AudioParams::REVERB_TAPS];
    int   reverbDelay[AudioParams::REVERB_TAPS];     // delay in milliseconds for reverb
    float reverbGain[AudioParams::REVERB_TAPS];

    // Bitcrush
    //float sampleCount    = params->SAMPLE_RATE
    int bitcrushCount  = 0;
    float bitcrushSample = 0.0f;

    float tremIncrement;   // precomputed 2*pi*f / sampleRate

    // Fuzz
    float fuzzSampleAvg = 0.0f;
    int fuzzSampleCount = (params->FUZZ_ATTACK / 1000) * params->SAMPLE_RATE;

    // Tone filter buffers
    float odToneBuffer[AudioParams::TONE_SIZE] = {};
    float distToneBuffer[AudioParams::TONE_SIZE] = {};
    float fuzzToneBuffer[AudioParams::TONE_SIZE] = {};

    // DC filter buffers
    float dcInputBuffer = 0.0f;
    float dcOutputBuffer = 0.0f;
};


extern EffectChoices effectChoice;

#endif
