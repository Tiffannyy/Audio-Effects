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
typedef float SAMPLE;

struct paTestData{
    int          frameIndex;
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
};

// Primarily for menu and callback functions
struct EffectChoices{
    bool norm   = false;
    bool trem   = false;
    bool delay  = false;
    bool reverb = false;
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

    const double PI     = 3.14159265358979323846;
    static constexpr int IN_CHANNELS   = 1;
    static constexpr int OUT_CHANNELS  = 2;
    static constexpr int SAMPLE_RATE   = 48000;
    
};

struct RtUserData {
    AudioParams *params;
    EffectChoices *effects;

    // delay
    std::vector<SAMPLE> delayBuffer;
    int delayIndex;
    int delaySize;

    // reverb
    std::vector<SAMPLE> reverbBuffer;
    int reverbSize;
    int reverbIndex[AudioParams::REVERB_TAPS];
    int reverbDelay[AudioParams::REVERB_TAPS];     // delay in milliseconds for reverb
    float reverbGain[AudioParams::REVERB_TAPS];

    double tremIncrement;   // precomputed 2*pi*f / sampleRate
};

extern EffectChoices effectChoice;
#endif
