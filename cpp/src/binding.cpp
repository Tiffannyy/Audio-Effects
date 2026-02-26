/*
 * binding.cpp
 * 25 February 2026
 * 
 * Tiffany Liu, Nathaniel Kalaw
 * 
 * Bridges the cpp and python files
*/

#include "../include/Engine.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE (engine, handler) {
    py::class_<Engine>(handler, "Engine")
        .def(py::init<const char*, const char*>())

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
        .def("read_peripherals", &Engine::readPeripherals);
}

