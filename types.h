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

// User Defined Data
typedef float SAMPLE;

struct paTestData{
    int          frameIndex;
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
};

// Primarily for menu and callback functions
struct EffectChoices{
    bool norm  = false;
    bool trem  = false;
    bool delay = false;
};

// Parameters to pass to callback functions
struct AudioParams{
    float TREM_FREQ     = 3.0;      // tremolo frequency (Hz). lower the freq, the slower the tremolo effect vice versa
    float TREM_DEPTH    = 0.3;      // tremolo depth. 0 has no effect, 1 has full effect
    float MIX           = 0.2;      // Mix between original and delayed signals
    const double PI     = 3.14159265358979323846;
    double tremPhase    = 0.1;
    static constexpr int DELAY_MS       = 1000 ;         // Delay in milliseconds
    static constexpr double FEEDBACK       = 0.8;        // Feedback amount (0 to 1)
    static constexpr int IN_CHANNELS   = 1;
    static constexpr int OUT_CHANNELS  = 2;

    static constexpr int SAMPLE_RATE    = 48000;
};

struct RtUserData {
    AudioParams *params;
    EffectChoices *effects;
    SAMPLE *delayBuffer;
    int delayBufferSize;
    unsigned int delayIndex;
    double tremIncrement;   // precomputed 2*pi*f / sampleRate
};

extern EffectChoices effectChoice;
#endif
