#pragma once

#include "eng-internal.hpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_gui_open(
        void) {

        gui_open(_eng_context->gui);
    }

    IFB_ENGINE_API void
    eng_gui_close(
        void) {

        gui_close(_eng_context->gui);
    }
};