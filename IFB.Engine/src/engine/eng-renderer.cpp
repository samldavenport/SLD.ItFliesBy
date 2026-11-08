#pragma once

#include "eng-renderer.hpp"

namespace ifb {

    IFB_ENGINE_API bool
    eng_renderer_is_grid_visible(
        void) {

        const bool is_visible = renderer_is_grid_visible();
        return(is_visible);
    }

    IFB_ENGINE_API void
    eng_renderer_set_grid_visible(
        const bool is_visible) {

        renderer_set_grid_visible(is_visible);
    }

    // the names are image assets, they all have to be the same size
    IFB_ENGINE_API bool
    eng_parallax_set(
        const cchar* front,
        const cchar* mid,
        const cchar* back) {

        assert(front && mid && back);

        // the renderer stores the layers back to front
        const bool did_set = renderer_set_parallax(back, mid, front);
        return(did_set);
    }
};
