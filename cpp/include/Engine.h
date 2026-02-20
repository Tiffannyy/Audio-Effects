/*
 * Engine.h
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 
 * 20 February 2026
*/

#pragma once

#include "audio.h"
#include <pybind11/pybind11.h>
#include <alsa/asoundlib.h>
#include <thread>
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
namespace py = pybind11;
class Engine {
public:
    Engine();
    ~Engine();

    void start();
    void stop();
    void setEffect(EffectChoices effect);
    void updateEffectChoice(void);

    // Parameter functions
    void setVolume(float v);
    void setMix(float v);
    void adjustTremFreq(int inc);
    void adjustTremDepth(int inc);
    //void adjustTremPhase(int inc);
    void adjustDelayMs(int inc);
    void adjustDelayFeedback(int inc);
    //void adjustReverbTaps(int inc);
    void adjustReverbDecay(int inc);
    void adjustBitcrushRate(int inc);
    void adjustBitcrushDepth(int inc);
    void adjustOdDrive(int inc);
    void adjustOdTone(int inc);
    void adjustDistDrive(int inc);
    void adjustDistTone(int inc);
    void adjustFuzzDrive(int inc);
    void adjustFuzzTone(int inc);

    // Peripheral functions
    void readPeripherals(void);
    void printEngineState(void); // for debugging

private:
    void streamLoop();
    std::thread audioThread;
    std::atomic<bool> running(false);

    std::mutex paramMutex;
    AudioParams audioParams;
    EffectChoices effectChoice;
    RtUserData userData;

    snd_pcm_t *inHandle = nullptr;
    snd_pcm_t *outHandle = nullptr;
    snd_pcm_uframes_t period{};
    snd_pcm_uframes_t buffer{};

    // Peripheral functions
    void readPotentiometers(void);
    void readEncoder(void);
    void readButton(void);

    // Menu variables
    MenuMode            menuMode = EFFECT_SELECTION_MODE;
    EffectSelection     effectSelection = NO_EFFECT;
    AudioParamSelection audioParamSelection;
}


PYBIND11_MODULE (engine, handler) {
    py::class_<Engine>(handler, "Engine")
        .def(py::init<>())

        .def("start", &Engine::start)
        .def("stop", &Engine::stop)
        .def("set_effect", &Engine::setEffect)

        .def("set_volume", &Engine::setVolume)
        .def("set_mix", &Engine::setMix)
        .def("adjust_trem_freq", &Engine::adjustTremFreq)
        .def("adjust_trem_depth", &Engine::adjustTremDepth)
        //.def("set_trem_phase", &Engine::setTremPhase)
        .def("adjust_delay_ms", &Engine::adjustDelayMs)
        .def("adjust_delay_feedback", &Engine::adjustDelayFeedback)
         //.def("set_reverb_taps", &Engine::setReverbTaps)
        .def("adjust_reverb_decay", &Engine::adjustReverbDecay)
        .def("adjust_bitcrush_rate", &Engine::adjustBitcrushRate)
        .def("adjust_bitcrush_depth", &Engine::adjustBitcrushDepth)
        .def("adjust_od_drive", &Engine::adjustOdDrive)
        .def("adjust_od_tone", &Engine::adjustOdTone)
        .def("adjust_dist_drive", &Engine::adjustDistDrive)
        .def("adjust_dist_tone", &Engine::adjustDistTone)
        .def("adjust_fuzz_drive", &Engine::adjustFuzzDrive)
        .def("adjust_fuzz_tone", &Engine::adjustFuzzTone)
        .def("read_peripherals", &Engine::readPeripherals)
}



