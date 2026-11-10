#pragma once

#include "eng-font.hpp"
#include "ifb.hpp"

namespace ifb {

    IFB_INTERNAL font*
    font_load_to_arena(
        const hnd_file  h_font_file,
        const hnd_arena h_arena) {

        assert(h_font_file != INVALID_HANDLE);
        assert(h_arena     != INVALID_HANDLE);
 
        return(NULL);
    } 
};
