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
#include <pybind11/stl.h>

namespace py = pybind11;


PYBIND11_MODULE (engine, handler) {
    py::class_<UIState>(handler, "UIState")
   	 .def_readonly("menuMode", &UIState::menuMode)
   	 .def_readonly("effectSel", &UIState::effectSel)
         .def_readonly("tremSel", &UIState::tremSel)
   	 .def_readonly("delaySel", &UIState::delaySel)
   	 .def_readonly("reverbSel", &UIState::reverbSel)
   	 .def_readonly("bitcrushSel", &UIState::bitcrushSel)
   	 .def_readonly("odSel", &UIState::odSel)
   	 .def_readonly("distSel", &UIState::distSel)
   	 .def_readonly("fuzzSel", &UIState::fuzzSel);

    py::class_<Engine>(handler, "Engine")
        .def(py::init<const char*, const char*>())

        .def("start", &Engine::start)
        .def("stop", &Engine::stop)

        .def("getParams", &Engine::getParams)
        .def("get_ui_state", &Engine::getUIState);
}

