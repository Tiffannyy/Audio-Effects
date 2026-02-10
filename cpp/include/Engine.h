/*
 * Engine.h
 * 
 * Tiffany Liu
 * 
 * 9 February 2026
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