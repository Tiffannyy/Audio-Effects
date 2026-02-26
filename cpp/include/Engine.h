/*
 * Engine.h
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 24 February 2026
*/

#pragma once

#include "audio.h"
//#include <pybind11/pybind11.h>
#include <alsa/asoundlib.h>
#include <thread>
#include <pthread.h>
#include <mutex>
#include <stdexcept>
#include <poll.h>

#define FRAMES_PER_BUFFER 256
#define BUFFER_MULT 4

#define EFFECT_SELECTION_MAX       7
#define TREMOLO_SELECTION_MAX      2
#define DELAY_SELECTION_MAX        2
#define REVERB_SELECTION_MAX       1
#define BITCRUSH_SELECTION_MAX     2
#define OVERDRIVE_SELECTION_MAX    2
#define DISTORTION_SELECTION_MAX   2
#define FUZZ_SELECTION_MAX         2

const bool DEBUG = 0;
const char* DEVICE_NAME = "hw:0,0";


// ============================================================
// [VARIABLES]


enum MenuMode {
    EFFECT_SELECTION_MODE = 0,   // User is choosing from effects
    AUDIO_PARAM_SELECTION_MODE,  // User is choosing parameters of an effect
    AUDIO_PARAM_VALUE_MODE       // User is adjusting a parameter value
};

enum EffectSelection {
    NO_EFFECT = 0,
    TREMOLO,
    DELAY,
    REVERB,
    BITCRUSH,
    OVERDRIVE,
    DISTORTION,
    FUZZ
};

enum TremoloSelection {
    TREMOLO_FREQ = 0, 
    TREMOLO_DEPTH,
    TREMOLO_BACK
};
    
enum DelaySelection {
    DELAY_MS = 0,
    DELAY_FEEDBACK,
    DELAY_BACK
};

enum ReverbSelection {
    REVERB_DECAY = 0,
    REVERB_BACK
};

enum BitcrushSelection {
    BITCRUSH_RATE = 0,
    BITCRUSH_DEPTH,
    BITCRUSH_BACK
};

enum OverdriveSelection {
    OVERDRIVE_DRIVE = 0,
    OVERDRIVE_TONE,
    OVERDRIVE_BACK
};

enum DistortionSelection {
    DISTORTION_DRIVE = 0,
    DISTORTION_TONE,
    DISTORTION_BACK
};

enum FuzzSelection {
    FUZZ_DRIVE = 0,
    FUZZ_TONE,
    FUZZ_BACK
};

struct AudioParamSelection {
    TremoloSelection    TREMOLO    = static_cast<TremoloSelection>(0);
    DelaySelection      DELAY      = static_cast<DelaySelection>(0);
    ReverbSelection     REVERB     = static_cast<ReverbSelection>(0);
    BitcrushSelection   BITCRUSH   = static_cast<BitcrushSelection>(0);
    OverdriveSelection  OVERDRIVE  = static_cast<OverdriveSelection>(0);
    DistortionSelection DISTORTION = static_cast<DistortionSelection>(0);
    FuzzSelection       FUZZ       = static_cast<FuzzSelection>(0);
};


// ============================================================
// [ENGINE]


// pybind module
//namespace py = pybind11;

class Engine {
public:
    Engine(const char* inputDevice, const char* outputDevice);
    ~Engine();

    void start();
    void stop();
    void setEffect(EffectChoices effect);
    
    // Parameter functions
    void setVolume(float v);
    void setMix(float v);
    void adjustTremFreq(int inc);
    void adjustTremDepth(int inc);
    void adjustDelayMs(int inc);
    void adjustDelayFeedback(int inc);
    void adjustReverbDecay(int inc);
    void adjustBitcrushRate(int inc);
    void adjustBitcrushDepth(int inc);
    void adjustOdDrive(int inc);
    void adjustOdTone(int inc);
    void adjustDistDrive(int inc);
    void adjustDistTone(int inc);
    void adjustFuzzDrive(int inc);
    void adjustFuzzTone(int inc);
    void updateEffectChoice(void);

    void readPeripherals();

private:
    void readEngineState();
    void readPotentiometers();
    void readEncoder();
    
    // Threads for individual tasks
    std::thread peripheralThread, guiThread, audioThread;
    void runPeripheralThread(void);
    void runGUIThread(void);
    void runStreamLoop();
    std::atomic<bool> running{false};

    std::mutex paramMutex;
    AudioParams audioParams;
    EffectChoices effectChoice;
    RtUserData userData;

    // ALSA variables
    snd_pcm_t *inHandle = nullptr;
    snd_pcm_t *outHandle = nullptr;
    snd_pcm_uframes_t period{};
    snd_pcm_uframes_t buffer{};

    // Menu variables
    MenuMode            menuMode = EFFECT_SELECTION_MODE;
    EffectSelection     effectSelection = NO_EFFECT;
    AudioParamSelection audioParamSelection;    
};

