#pragma once

#include "eng-internal.hpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_gui_open(
        void) {

        gui_open();
    }

    IFB_ENGINE_API void
    eng_gui_close(
        void) {

        gui_close();
    }
};