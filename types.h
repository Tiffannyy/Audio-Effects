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


struct paTestData{
    int          frameIndex;
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
};


// Primarily for menu and callback functions
struct EffectChoices{
    bool norm     = false;
    bool trem     = false;
    bool delay    = false;
    bool reverb   = false;
    bool bitcrush = false;
};


// Parameters to pass to callback functions
struct AudioParams{

    float MIX           = 0.3;      // Mix between original and delayed signals

    // Tremolo
    float TREM_FREQ     = 4.0;      // tremolo frequency (Hz). lower the freq, the slower the tremolo effect vice versa
    float TREM_DEPTH    = 0.5;      // tremolo depth. 0 has no effect, 1 has full effect
    double tremPhase    = 0.1;

    // Delay
    static constexpr int DELAY_MS       = 500;      // delay in milliseconds
    static constexpr double FEEDBACK    = 0.2;      // feedback amount (0 to 1)   -  for delay

    // Reverb
    static const int REVERB_TAPS        = 5;        // number of delay taps for reverb
    static constexpr float reverbDecay  = 0.6;      // decay factor for reverb

    // Bitcrush
    int DOWNSAMPLE_RATE = 12000;     // Rate to "resample" input signal (Hz) (Must NOT exceed sample rate)
    int BIT_DEPTH       = 8;        // Amount of bits to "quantize" sample amplitude

    const double PI     = 3.14159265358979323846;
    static constexpr int CHANNELS = 2;
    static constexpr unsigned int SAMPLE_RATE   = 48000;
};


struct RtUserData {

    AudioParams *params;
    EffectChoices *effects;

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
    //double sampleCount    = params->SAMPLE_RATE
    double bitcrushCount  = 0.0;
    SAMPLE bitcrushSample = 0.0f;

    double tremIncrement;   // precomputed 2*pi*f / sampleRate
};


extern EffectChoices effectChoice;

#endif
