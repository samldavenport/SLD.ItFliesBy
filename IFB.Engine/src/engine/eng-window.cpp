#pragma once

#include "ifb.hpp"
#include "eng-context.hpp"
#include "renderer-internal.hpp"

namespace ifb {

    IFB_ENGINE_API void
    eng_window_open(
        void) {

        
    }

    IFB_ENGINE_API void
    eng_window_did_close(
        void) {

    }

    IFB_ENGINE_API void
    eng_window_set_pos(
        const u32 pos_x,
        const u32 window_pos_y) {

    }

    IFB_ENGINE_API void
    eng_window_set_size(
        const u32 width,
        const u32 height) {

        renderer_set_viewport_dimensions(
            width,
            height
        );

    }
};
