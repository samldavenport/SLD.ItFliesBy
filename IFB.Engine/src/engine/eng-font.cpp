#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-font.hpp"

namespace ifb {

    IFB_ENGINE_API font*
    eng_font_load_to_arena(
        const hnd_file  h_font_file,
        const hnd_arena h_arena) {

        assert(h_font_file != INVALID_HANDLE);
        assert(h_arena     != INVALID_HANDLE);

        font* f = font_load_to_arena(h_font_file, h_arena); 

        return(f);
    }
};
