/*
 * Engine.h
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 
 * 12 February 2026
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
#define TREMOLO_SELECTION_MAX      3
#define DELAY_SELECTION_MAX        2
#define REVERB_SELECTION_MAX       2
#define BITCRUSH_SELECTION_MAX     2
#define OVERDRIVE_SELECTION_MAX    2
#define DISTORTION_SELECTION_MAX   2
#define FUZZ_SELECTION_MAX         2

const bool DEBUG = 0;
const char* DEVICE_NAME = "hw:0,0";

namespace py = pybind11;
class Engine {
public:
    Engine();
    ~Engine();

    void start();
    void stop();

    void setEffect(EffectChoices effect);

    void setMix(float v);

    void setTremFreq(float v);
    void setTremDepth(float v);
    void setTremPhase(float v);

    void setDelay(int v);
    void setFeedback(float v);

    void setReverbTaps(int v);
    void setReverbDecay(float v);

    void setDownSampleRate(int v);
    void setBitDepth(int v);

    void setOdDrive(float v);
    void setOdTone(float v);

    void setDistDrive(float v);
    void setDistTone(float v);

    void setFuzzDrive(float v);
    void setFuzzTone(float v);

    void readPeripherals(void);

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

}

PYBIND11_MODULE (engine, handler) {
    py::class_<Engine>(handler, "Engine")
        .def(py::init<>())

        .def("start", &Engine::start)
        .def("stop", &Engine::stop)
        .def("set_effect", &Engine::setEffect)
        .def("set_mix", &Engine::setMix)
        .def("set_trem_freq", &Engine::setTremFreq)
        .def("set_trem_depth", &Engine::setTremDepth)
        .def("set_trem_phase", &Engine::setTremPhase)
        .def("set_delay", &Engine::setDelay)
        .def("set_feedback", &Engine::setFeedback)
        .def("set_reverb_taps", &Engine::setReverbTaps)
        .def("set_reverb_decay", &Engine::setReverbDecay)
        .def("set_downsample_rate", &Engine::setDownSampleRate)
        .def("set_bit_depth", &Engine::setBitDepth)
        .def("set_od_drive", &Engine::setOdDrive)
        .def("set_od_tone", &Engine::setOdTone)
        .def("set_dist_drive", &Engine::setDistDrive)
        .def("set_dist_tone", &Engine::setDistTone)
        .def("set_fuzz_drive", &Engine::setFuzzDrive)
        .def("set_fuzz_tone", &Engine::setFuzzTone)
}


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
    TREMOLO_PHASE,
    TREMOLO_BACK
};
    
enum DelaySelection {
    DELAY_MS = 0,
    DELAY_FEEDBACK,
    DELAY_BACK
};

enum ReverbSelection {
    REVERB_TAPS = 0,
    REVERB_DECAY,
    REVERB_BACK
};

enum BitcrushSelection {
    BITCRUSH_DOWNSAMPLE = 0,
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
    TremoloSelection    TREMOLO    = TREMOLO_FREQ;
    DelaySelection      DELAY      = DELAY_MS;
    ReverbSelection     REVERB     = REVERB_TAPS;
    BitcrushSelection   BITCRUSH   = BITCRUSH_DOWNSAMPLE;
    OverdriveSelection  OVERDRIVE  = OVERDRIVE_DRIVE;
    DistortionSelection DISTORTION = DISTORTION_DRIVE;
    FuzzSelection       FUZZ       = FUZZ_DRIVE;
};

extern MenuMode            MENU_MODE;
extern EffectSelection     EFFECT_SELECTION;
extern AudioParamSelection AUDIO_PARAM_SELECTION;
